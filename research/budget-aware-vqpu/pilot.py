"""Paired comparison of region exploration and random restarts in QSteed."""
import csv
import json
import math
from pathlib import Path
import os
import random
import statistics
import sys
import tempfile
import time
import types

import networkx as nx
import numpy as np
from scipy.stats import spearmanr

BASIS = ["cz", "rx", "ry", "rz", "h"]
REGIONS = 8
RESTARTS = 8


def substrates():
    fixture = json.loads(Path("tests/dongling.json").read_text())
    dongling = nx.Graph()
    for pair, props in fixture["topological_structure"].items():
        f = props["CZ"]["fidelity"]
        if 0 < f <= 1:
            dongling.add_edge(*pair.split("_"), fidelity=f)
    grid = nx.convert_node_labels_to_integers(nx.grid_2d_graph(6, 6))
    rng = random.Random(427)
    for u, v in sorted(grid.edges):
        grid[u][v]["fidelity"] = rng.uniform(0.94, 0.997)
    return {"dongling_fixture": dongling, "synthetic_grid": grid}


def candidate_regions(chip, width):
    """Grow one connected region per seed edge and rank unique regions by the original product score."""
    regions = {}
    for seed in chip.edges:
        nodes = set(seed)
        while len(nodes) < width:
            frontier = [
                (attrs["fidelity"], v) for u in nodes
                for v, attrs in chip[u].items() if v not in nodes
            ]
            if not frontier:
                break
            nodes.add(max(frontier)[1])
        if len(nodes) == width:
            key = tuple(sorted(nodes))
            regions[key] = chip.subgraph(key)
    ranked = sorted(
        regions.items(),
        key=lambda pair: (-sum(math.log(f) for _, _, f in pair[1].edges.data("fidelity")), pair[0]),
    )
    assert len(ranked) >= REGIONS
    return ranked[:REGIONS], len(ranked)


def make_circuit(width, family, seed):
    from quafu import QuantumCircuit

    rng = random.Random(seed)
    circ = QuantumCircuit(width)
    for q in range(width):
        circ.ry(q, rng.uniform(-2, 2))
    for _ in range(3):
        if family == "ring":
            pairs = [(i, (i + 1) % width) for i in range(width)]
        elif family == "star":
            pairs = [(0, i) for i in range(1, width)]
        else:
            pairs = [tuple(rng.sample(range(width), 2)) for _ in range(width)]
        rng.shuffle(pairs)
        for u, v in pairs:
            circ.cz(u, v)
        for q in range(width):
            circ.rx(q, rng.uniform(-2, 2))
    circ.measure(list(range(width)), list(range(width)))
    return circ


def simulate(circuit, state):
    tensor = state.reshape((2,) * circuit.num)
    for gate in circuit.gates:
        axes = list(gate.pos)
        remainder = [q for q in range(circuit.num) if q not in axes]
        order = axes + remainder
        front = tensor.transpose(order).reshape(2 ** len(axes), -1)
        tensor = (gate.matrix @ front).reshape((2,) * circuit.num).transpose(np.argsort(order))
    return tensor


def compile_trial(circ, region, trial, state, expected):
    from qsteed.backends.backend import Backend
    from qsteed.passes.mapping.layout.sabre_layout import SabreLayout
    from qsteed.passes.model import Model
    from qsteed.passes.optimization.optimization_combine import GateCombineOptimization
    from qsteed.passes.unroll.unroll_to_basis import UnrollToBasis

    started = time.perf_counter()
    random.seed(9000 + trial)
    ordered_nodes, subgraph = region
    to_virtual = {name: i for i, name in enumerate(ordered_nodes)}
    fidelity = {}
    for u, v, f in subgraph.edges.data("fidelity"):
        fidelity[to_virtual[u], to_virtual[v]] = f
        fidelity[to_virtual[v], to_virtual[u]] = f
    backend = Backend(qubits_num=len(ordered_nodes),
                      coupling_list=[(*edge, f) for edge, f in sorted(fidelity.items())],
                      basis_gates=BASIS)
    model = Model(backend=backend)
    layout = SabreLayout(heuristic="mixture", max_iterations=3)
    layout.set_model(model)
    compiled = GateCombineOptimization().run(UnrollToBasis(BASIS).run(layout.run(circ)))
    elapsed = time.perf_counter() - started

    initial = model.get_layout()["initial_layout"].v2p
    final = model.get_layout()["final_layout"].v2p
    assert compiled.measures == {final[q]: c for q, c in circ.measures.items()}
    physical_state = state.reshape((2,) * circ.num).transpose(np.argsort(list(initial[q] for q in range(circ.num))))
    actual = simulate(compiled, physical_state).transpose([final[q] for q in range(circ.num)])
    assert abs(np.vdot(expected, actual)) > 1 - 1e-9

    layers = [0] * compiled.num
    cost = 0.0
    cz_count = 0
    for gate in compiled.gates:
        name = gate.name.lower()
        bits = tuple(gate.pos)
        assert name in BASIS and len(bits) in (1, 2)
        depth = 1 + max(layers[b] for b in bits)
        for b in bits:
            layers[b] = depth
        if name == "cz":
            cost -= math.log(fidelity[bits])
            cz_count += 1
        else:
            cost -= math.log(0.996)
    depth = max(layers)
    cost -= depth * math.log(0.995)
    return {"cost": cost, "seconds": elapsed, "native_cz": cz_count, "depth": depth}


def evaluate():
    rows = []
    for device, graph in substrates().items():
        print(device, graph.number_of_nodes(), graph.number_of_edges(), flush=True)
        for width in (5, 8, 10):
            regions, pool = candidate_regions(graph, width)
            for family_index, family in enumerate(("ring", "star", "random")):
                for instance in range(2):
                    seed = 1000 * width + 100 * family_index + instance
                    circ = make_circuit(width, family, seed)
                    rng = np.random.default_rng(seed)
                    state = rng.normal(size=2**width) + 1j * rng.normal(size=2**width)
                    state /= np.linalg.norm(state)
                    expected = simulate(circ, state)
                    for region_index, region in enumerate(regions):
                        for trial in range(RESTARTS):
                            rows.append({
                                "device": device, "width": width, "family": family,
                                "instance": instance, "pool": pool, "region": region_index,
                                "physical_nodes": "|".join(map(str, region[0])), "trial": trial,
                                **compile_trial(circ, region, trial, state, expected),
                            })
            print(device, width, "complete", flush=True)
    return rows


def summarize(rows):
    grouped = {}
    for row in rows:
        key = (row["device"], row["width"], row["family"], row["instance"])
        grouped.setdefault(key, {})[row["region"], row["trial"]] = row
    records = []
    for key, trials in grouped.items():
        top = sorted(range(4), key=lambda r: trials[r, 0]["cost"])[:2]
        policies = {
            "one": [(0, 0)],
            "restart": [(0, i) for i in range(8)],
            "breadth": [(r, 0) for r in range(8)],
            "fixed_4x2": [(r, t) for r in range(4) for t in range(2)],
            "screen_4_then_refine_2": [(r, 0) for r in range(4)] + [(r, t) for r in top for t in (1, 2)],
            "sample_oracle": list(trials),
        }
        best = {name: min(trials[k]["cost"] for k in keys) for name, keys in policies.items()}
        times = {name: sum(trials[k]["seconds"] for k in keys) for name, keys in policies.items()}
        probe = [trials[r, 0]["cost"] for r in range(8)]
        independent = [min(trials[r, t]["cost"] for t in range(1, 8)) for r in range(8)]
        rho = float(spearmanr(probe, independent).statistic)
        records.append({"key": key, "cost": best, "seconds": times, "rho": rho,
                        "best_region": min(trials, key=lambda k: trials[k]["cost"])[0]})
    return records


def main():
    out = Path(sys.argv[1] if len(sys.argv) > 1 else ".")
    with tempfile.TemporaryDirectory(prefix="qsteed-research-") as tmp:
        config = Path(tmp) / "config.ini"
        config.write_text(Path("qsteed/config/config.ini").read_text().replace(
            "~/QSteed/qsteed.db", str(Path(tmp) / "pilot.db")))
        os.environ["MPLBACKEND"] = "Agg"
        os.environ["MPLCONFIGDIR"] = str(Path(tmp) / "matplotlib")
        module = types.ModuleType("qsteed.config.get_config")
        module.get_config = lambda user_config_file=None: user_config_file or str(config)
        sys.modules[module.__name__] = module
        rows = evaluate()
    with (out / "trials.csv").open("w", newline="") as handle:
        writer = csv.DictWriter(handle, fieldnames=list(rows[0]), lineterminator="\n")
        writer.writeheader()
        writer.writerows(rows)
    records = summarize(rows)
    (out / "summary.json").write_text(json.dumps(records, indent=2, allow_nan=False) + "\n")
    print(len(rows), "validated trials")
    for device in ("all", *substrates()):
        items = [r for r in records if device == "all" or r["key"][0] == device]
        print(device, len(items), "sample best outside first region",
              sum(r["best_region"] != 0 for r in items),
              "median probe-vs-independent rho", round(statistics.median(r["rho"] for r in items), 3))
        for p in items[0]["cost"]:
            delta = [(r["cost"]["restart"] - r["cost"][p]) / r["cost"]["restart"] for r in items]
            print(p, "median % lower cost", round(100 * statistics.median(delta), 2),
                  "better/tied/worse", [sum(x > 1e-9 for x in delta),
                                       sum(abs(x) <= 1e-9 for x in delta),
                                       sum(x < -1e-9 for x in delta)],
                  "median CPU-serial wall ms", round(1000 * statistics.median(r["seconds"][p] for r in items), 1))


if __name__ == "__main__":
    main()
