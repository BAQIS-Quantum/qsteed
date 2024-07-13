# This code is part of QSteed.
#
# (C) Copyright 2024 Beijing Academy of Quantum Information Sciences
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#   http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import warnings

import matplotlib.pyplot as plt


def plot_probabilities(sampling_dict: dict, complete_bitstring: bool = True, plot_type: str = "probability"):
    """
    Plot the probabilities or counts from execute results.

    Args:
        sampling_dict (dict): Dictionary with bitstring keys and counts/probabilities as values.
        complete_bitstring (bool): If True, include all possible bitstrings in the plot,
                                    even if they have zero counts/probabilities.
        plot_type (str): "probability" to plot probabilities, "count" to plot raw counts.
    """
    if sampling_dict is None:
        raise ValueError("sampling_dict cannot be None")

    # Detect if the sampling_dict contains counts or probabilities
    values = list(sampling_dict.values())
    is_probability = all(0 <= abs(value) <= 1.9 for value in values) and abs(sum(values) - 1) < 9e-1

    qubits = len(list(sampling_dict.keys())[0])
    total_counts = sum(sampling_dict.values())

    if complete_bitstring:
        all_keys = [format(i, f'0{qubits}b') for i in range(2 ** qubits)]
        complete_dict = {key: sampling_dict.get(key, 0) for key in all_keys}
    else:
        complete_dict = sampling_dict

    # Sort the dictionary by key
    sorted_items = sorted(complete_dict.items())
    bitstrs = [item[0] for item in sorted_items]
    values = [item[1] for item in sorted_items]

    probs = values
    ylabel = "Probabilities"
    if plot_type == "probability":
        if not is_probability:
            probs = [value / total_counts for value in values]
    elif plot_type == "count":
        if is_probability:
            ylabel = "Probabilities"
            warnings.warn("Input sampling_dict data is probabilities.", Warning)
        else:
            ylabel = "Counts"

    plt.figure()
    plt.bar(range(len(probs)), probs, tick_label=bitstrs)
    plt.xticks(rotation=70)
    plt.xlabel("Bitstrings")
    plt.ylabel(ylabel)
    plt.tight_layout()
    plt.show()
