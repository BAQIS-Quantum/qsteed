"""Replay logged serial compilation durations under fixed budgets."""
import csv
from pathlib import Path
from statistics import median
import sys

root = Path(sys.argv[1]) if len(sys.argv) > 1 else Path(__file__).parent
grouped = {}
with (root / "trials.csv").open() as handle:
    for row in csv.DictReader(handle):
        key = tuple(row[k] for k in ("device", "width", "family", "instance"))
        trial = (int(row["region"]), int(row["trial"]))
        grouped.setdefault(key, {})[trial] = float(row["seconds"]), float(row["cost"])

for budget in (0.1, 0.25, 0.5):
    scores = []
    for trials in grouped.values():
        top = sorted(range(4), key=lambda r: trials[r, 0][1])[:2]
        orders = {
            "restart": [(0, i) for i in range(8)],
            "breadth": [(r, 0) for r in range(8)],
            "fixed_4x2": [(r, t) for t in range(2) for r in range(4)],
            "screen": [(r, 0) for r in range(4)] + [(r, t) for t in (1, 2) for r in top],
        }
        summary = {}
        for policy, order in orders.items():
            elapsed = 0.0
            costs = []
            for trial in order:
                seconds, cost = trials[trial]
                elapsed += seconds
                if elapsed > budget:
                    break
                costs.append(cost)
            if costs:
                summary[policy] = min(costs)
        if len(summary) == len(orders):
            scores.append(summary)
    print(f"{budget:.2f} seconds, common completed cases {len(scores)}/{len(grouped)}")
    for policy in ("breadth", "fixed_4x2", "screen"):
        values = [(s["restart"] - s[policy]) / s["restart"] for s in scores]
        print(policy, f"{100 * median(values):+.2f}%",
              [sum(x > 1e-9 for x in values),
               sum(abs(x) <= 1e-9 for x in values),
               sum(x < -1e-9 for x in values)])
