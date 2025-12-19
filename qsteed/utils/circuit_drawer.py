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


class DrawConfig:
    def __init__(self):
        # Unicode box drawing characters (Qiskit style)
        self.h_wire = "─"
        self.v_wire = "│"
        self.top_left = "┌"
        self.top_right = "┐"
        self.bot_left = "└"
        self.bot_right = "┘"
        self.left_connect = "┤"
        self.right_connect = "├"
        self.double_h_wire = "═"
        self.double_v_wire = "║"
        self.measure_v_wire = "╥"  # Special character for measurement
        # self.control_dot = "■"
        self.control_dot = "◉"
        self.swap_symbol = "X"
        self.barrier = "░"
        self.h_wire_cross = "╪"
        self.top_t = "┴"
        self.bot_t = "┬"
        self.measure_cross = "╫"
        self.plus_cross = "┼"

class CircuitTextDrawer:
    """ ASCII text quantum circuit drawer"""

    def __init__(self, circuit, config=None):
        self.circuit = circuit
        self.config = config or DrawConfig()
        self.num_qubits = circuit.num_qubits()
        self.num_clbits = circuit.num_clbits()
        
        self.grid = []
        self.qubit_y = {}
        self.clbit_y = -1
        self.measure_num_y = -1
        self.measurement_info = []

    def _get_gate_width(self, gate):
        """Calculate the horizontal width of a gate."""
        if not gate:
            return 1

        name = gate.name.upper()

        if name in ["CX", "CNOT", "CCX", "TOFFOLI"]:
            return 5
        if name == "CZ":
            return 3
        if name == "SWAP":
            return 3

        symbol = self._get_gate_symbol(gate)

        # Check if it's a measurement
        is_measure = False
        if hasattr(gate, 'is_measurement') and callable(gate.is_measurement):
            is_measure = gate.is_measurement()
        elif name in ['MEASURE', 'MEASUREMENT']:
            is_measure = True

        if len(gate.qubits) == 1 or is_measure:
            return len(symbol) + 4  # e.g., ┤ H ├

        # Generic multi-qubit
        return max(len(symbol), 1) + 4
        
    def _initialize_grid(self, width, qubit_map):
        y = 0
        for i in range(self.num_qubits):
            if i in qubit_map:
                self.qubit_y[i] = y + 1
                y += 2
        
        total_rows = y + 1
        if self.num_clbits > 0:
            total_rows += 2
            self.clbit_y = y + 1
            self.measure_num_y = y + 2
            
        self.grid = [[' '] * width for _ in range(total_rows)]
        
        # Labels and wires
        max_q_label_width = len(f"q_{self.num_qubits-1}: ") if self.num_qubits > 0 else 0
        
        for q_idx, y_pos in self.qubit_y.items():
            label = f"q_{q_idx}: ".ljust(max_q_label_width)
            self.grid[y_pos][:len(label)] = list(label)
            for i in range(len(label), width):
                self.grid[y_pos][i] = self.config.h_wire
        
        if self.num_clbits > 0:
            label = f"c: {self.num_clbits}/".ljust(max_q_label_width)
            self.grid[self.clbit_y][:len(label)] = list(label)
            for i in range(len(label), width):
                self.grid[self.clbit_y][i] = self.config.double_h_wire
                
        return max_q_label_width

    def _draw_box(self, y_pos, symbol, col, gate_width, layer_width):
        """Helper to draw a complete 3-line gate box, centered in layer_width."""
        inner_width = gate_width - 2
        left_pad = (layer_width - gate_width) // 2
        box_start = col + left_pad

        # Box lines
        top_line = self.config.top_left + self.config.h_wire * inner_width + self.config.top_right
        mid_line = self.config.left_connect + symbol.center(inner_width) + self.config.right_connect
        bot_line = self.config.bot_left + self.config.h_wire * inner_width + self.config.bot_right

        # Draw top border (check for connection with box above)
        for i, char in enumerate(top_line):
            x = box_start + i
            existing = self.grid[y_pos - 1][x]

            # Smart replacement based on what we're drawing and what exists
            if char == self.config.top_left:  # Want to draw ┌
                if existing == self.config.bot_left:  # Existing is └
                    self.grid[y_pos - 1][x] = self.config.right_connect  # Draw ├ (left side vertical connection)
                elif existing == self.config.bot_right:  # Existing is ┘
                    self.grid[y_pos - 1][x] = self.config.top_t  # Draw ┴
                elif existing == self.config.h_wire:  # Existing is ─
                    self.grid[y_pos - 1][x] = self.config.bot_t  # Draw ┬
                else:
                    self.grid[y_pos - 1][x] = char
            elif char == self.config.top_right:  # Want to draw ┐
                if existing == self.config.bot_right:  # Existing is ┘
                    self.grid[y_pos - 1][x] = self.config.left_connect  # Draw ┤ (right side vertical connection)
                elif existing == self.config.bot_left:  # Existing is └
                    self.grid[y_pos - 1][x] = self.config.top_t  # Draw ┴
                elif existing == self.config.h_wire:  # Existing is ─
                    self.grid[y_pos - 1][x] = self.config.bot_t  # Draw ┬
                else:
                    self.grid[y_pos - 1][x] = char
            else:
                # For middle horizontal lines
                if existing == self.config.bot_left or existing == self.config.bot_right:
                    self.grid[y_pos - 1][x] = self.config.top_t  # Draw ┴
                else:
                    self.grid[y_pos - 1][x] = char

        # Draw middle line (on the qubit wire)
        for i in range(col, col + layer_width):
            if i < box_start:
                self.grid[y_pos][i] = self.config.h_wire
            elif i < box_start + gate_width:
                self.grid[y_pos][i] = mid_line[i - box_start]
            else:
                self.grid[y_pos][i] = self.config.h_wire

        # Draw bottom border
        for i, char in enumerate(bot_line):
            self.grid[y_pos + 1][box_start + i] = char

        # Return the box info
        return {
            'left': box_start,
            'right': box_start + gate_width - 1,
            'center': box_start + gate_width // 2
        }

    def _draw_multi_qubit_box(self, y_coords, symbol, col, gate_width, layer_width):
        """Draw a large box spanning multiple qubits."""
        inner_width = gate_width - 2
        left_pad = (layer_width - gate_width) // 2
        box_start = col + left_pad

        # Sort y coordinates to get top and bottom
        y_sorted = sorted(y_coords)
        y_min, y_max = y_sorted[0], y_sorted[-1]

        # Get all qubit lines between min and max (including unused ones)
        all_y_in_range = []
        for q_idx in sorted(self.qubit_y.keys()):
            y = self.qubit_y[q_idx]
            if y_min <= y <= y_max:
                all_y_in_range.append(y)

        # Top border line
        top_line = self.config.top_left + self.config.h_wire * inner_width + self.config.top_right
        bot_line = self.config.bot_left + self.config.h_wire * inner_width + self.config.bot_right

        # Draw top border (with connection checking)
        for i, char in enumerate(top_line):
            x = box_start + i
            existing = self.grid[y_min - 1][x]
            if existing == self.config.bot_left or existing == self.config.bot_right:
                self.grid[y_min - 1][x] = self.config.top_t  # ┴
            elif (char == self.config.top_left or char == self.config.top_right) and existing == self.config.h_wire:
                self.grid[y_min - 1][x] = self.config.bot_t  # ┬
            else:
                self.grid[y_min - 1][x] = char

        # Draw each qubit line in range
        num_qubits = len(y_sorted)
        # Calculate the overall middle position for 2-qubit gate name
        overall_mid = (y_min + y_max) // 2

        for y in all_y_in_range:
            if y in y_sorted:
                # This qubit is used by the gate
                idx = y_sorted.index(y)

                if num_qubits == 1:
                    content = ("0 " + symbol).ljust(inner_width)
                elif num_qubits == 2:
                    if idx == 0:
                        content = "0".ljust(inner_width)
                    else:
                        content = "1".ljust(inner_width)
                else:
                    mid_idx = num_qubits // 2
                    if idx == 0:
                        content = "0".ljust(inner_width)
                    elif idx == num_qubits - 1:
                        content = str(num_qubits - 1).ljust(inner_width)
                    elif idx == mid_idx:
                        content = symbol.center(inner_width)
                    else:
                        content = " " * inner_width
            else:
                # This qubit is not used by the gate
                # For 2-qubit gates, if this is the middle position, show gate name
                if num_qubits == 2 and y == overall_mid:
                    content = symbol.center(inner_width)
                else:
                    content = " " * inner_width

            mid_line = self.config.left_connect + content + self.config.right_connect

            # Draw horizontal wires and box content
            for i in range(col, col + layer_width):
                if i < box_start:
                    self.grid[y][i] = self.config.h_wire
                elif i < box_start + gate_width:
                    self.grid[y][i] = mid_line[i - box_start]
                else:
                    self.grid[y][i] = self.config.h_wire

        # Draw vertical borders between all qubit lines in range
        for i in range(len(all_y_in_range) - 1):
            y_curr = all_y_in_range[i]
            y_next = all_y_in_range[i + 1]

            for between_y in range(y_curr + 1, y_next):
                self.grid[between_y][box_start] = self.config.v_wire
                self.grid[between_y][box_start + gate_width - 1] = self.config.v_wire

                # For 2-qubit gates, show gate name at the overall middle position if it's in empty space
                if num_qubits == 2 and between_y == overall_mid:
                    gate_content = symbol.center(inner_width)
                    for j, ch in enumerate(gate_content):
                        self.grid[between_y][box_start + 1 + j] = ch

        # Draw bottom border
        for i, char in enumerate(bot_line):
            self.grid[y_max + 1][box_start + i] = char

        return {
            'left': box_start,
            'right': box_start + gate_width - 1,
            'center': box_start + gate_width // 2
        }

    def _draw_control_dot(self, y_pos, col, layer_width):
        """Helper to draw a control dot centered in layer."""
        center = layer_width // 2
        dot_line = self.config.h_wire * center + self.config.control_dot + self.config.h_wire * (layer_width - center - 1)
        self.grid[y_pos][col:col + layer_width] = list(dot_line)

    def _draw_swap_symbol(self, y_pos, col, layer_width):
        """Helper to draw a swap symbol centered in layer."""
        center = layer_width // 2
        swap_line = self.config.h_wire * center + self.config.swap_symbol + self.config.h_wire * (layer_width - center - 1)
        self.grid[y_pos][col:col + layer_width] = list(swap_line)

    def _draw_vertical_connection(self, y1, y2, x, box_info_list):
        """Draw vertical connection line."""
        for y in range(min(y1, y2), max(y1, y2) + 1):
            # Skip the start and end points
            if y == y1 or y == y2:
                continue

            current_char = self.grid[y][x]

            # Check if there's a box below (this line is the top border of a box)
            box_below = None
            for box in box_info_list:
                if box['y'] == y + 1 and box['left'] <= x <= box['right']:
                    box_below = box
                    break

            if current_char == ' ':
                # Empty space - check if we need to add top/bottom border of a box
                box_above = None
                for box in box_info_list:
                    if box['y'] == y - 1 and box['left'] <= x <= box['right']:
                        box_above = box
                        break

                if box_below:
                    # Add top border with T-junction
                    for bx in range(box_below['left'], box_below['right'] + 1):
                        if bx == box_below['left']:
                            self.grid[y][bx] = self.config.top_left
                        elif bx == box_below['right']:
                            self.grid[y][bx] = self.config.top_right
                        elif bx == x:
                            self.grid[y][bx] = self.config.top_t  # ┴
                        else:
                            self.grid[y][bx] = self.config.h_wire
                elif box_above:
                    # This should already have bottom border from _draw_box
                    # Just add vertical line at connection point if needed
                    if self.grid[y][x] == self.config.h_wire:
                        self.grid[y][x] = self.config.bot_t  # ┬
                    else:
                        self.grid[y][x] = self.config.v_wire
                else:
                    self.grid[y][x] = self.config.v_wire
            elif current_char == self.config.h_wire:
                # On a horizontal wire - check if it's a box border
                if box_below:
                    # This is the top border of a box below
                    self.grid[y][x] = self.config.top_t  # ┴
                else:
                    # Regular wire crossing (on a qubit line)
                    self.grid[y][x] = self.config.plus_cross  # ┼
            elif current_char == self.config.top_left or current_char == self.config.top_right:
                # Top corner of a box
                self.grid[y][x] = self.config.top_t  # ┴
            elif current_char == self.config.bot_left or current_char == self.config.bot_right:
                # Bottom corner of a box
                self.grid[y][x] = self.config.bot_t  # ┬
                
    def _draw_measurement_line(self, q_y, c_y, x):
        for y in range(q_y + 1, c_y):
            if self.grid[y][x] == self.config.h_wire:
                self.grid[y][x] = self.config.measure_cross
            elif self.grid[y][x] == ' ':
                self.grid[y][x] = self.config.double_v_wire
        self.grid[q_y+1][x] = self.config.measure_v_wire
        self.grid[c_y][x] = '╩'
        
    def _draw_layer(self, col_data, current_col):
        layer_width = col_data['width']
        connections = []  # Store connection info for second pass
        box_info_list = []  # Store all box positions

        # Pass 1: Draw all instructions
        for inst in col_data['gates']:
            if inst is None:
                continue

            q_indices = inst.qubits
            y_coords = [self.qubit_y[q] for q in q_indices]
            inst_width = self._get_gate_width(inst)

            # First level dispatch: by instruction type
            if inst.is_measurement():
                self._draw_measurement_instruction(inst, y_coords, current_col, inst_width, layer_width, box_info_list, connections)
            elif inst.is_barrier():
                self._draw_barrier_instruction(inst, y_coords, current_col, layer_width)
            elif inst.is_gate():
                # Second level dispatch: by gate name (inside _draw_gate_instruction)
                self._draw_gate_instruction(inst, y_coords, current_col, inst_width, layer_width, box_info_list, connections)

        # Pass 2: Draw connections
        for conn in connections:
            if conn['type'] == 'measurement':
                self._draw_measurement_line(conn['y'], self.clbit_y, conn['x'])
            elif conn['type'] == 'vertical':
                self._draw_vertical_connection(conn['y1'], conn['y2'], conn['x'], box_info_list)

    def _add_box_info(self, box_info, y, box_info_list):
        """Helper to add box info to list"""
        box_info_list.append({'y': y, **box_info})

    def _add_vertical_connection(self, y1, y2, x, connections):
        """Helper to add vertical connection"""
        connections.append({'type': 'vertical', 'y1': y1, 'y2': y2, 'x': x})

    def _draw_controlled_gate(self, inst, control_qubits, target_qubit, target_symbol,
                              current_col, inst_width, layer_width, box_info_list, connections):
        """Generic function to draw controlled gates (CNOT, Toffoli, etc.)"""
        center_x = current_col + layer_width // 2

        # Draw control dots
        for ctrl_q in control_qubits:
            ctrl_y = self.qubit_y[ctrl_q]
            self._draw_control_dot(ctrl_y, current_col, layer_width)

        # Draw target box
        targ_y = self.qubit_y[target_qubit]
        box_info = self._draw_box(targ_y, target_symbol, current_col, inst_width, layer_width)
        self._add_box_info(box_info, targ_y, box_info_list)

        # Add vertical connection
        all_qubits = control_qubits + [target_qubit]
        y_coords = [self.qubit_y[q] for q in all_qubits]
        self._add_vertical_connection(min(y_coords), max(y_coords), center_x, connections)

    def _draw_two_qubit_symmetric_gate(self, inst, symbol_func, current_col, layer_width, connections):
        """Generic function for symmetric two-qubit gates (CZ, SWAP)"""
        y1, y2 = self.qubit_y[inst.qubits[0]], self.qubit_y[inst.qubits[1]]
        center_x = current_col + layer_width // 2

        # Draw symbols on both qubits
        symbol_func(y1, current_col, layer_width)
        symbol_func(y2, current_col, layer_width)

        # Add vertical connection
        self._add_vertical_connection(y1, y2, center_x, connections)

    def _draw_measurement_instruction(self, inst, y_coords, current_col, inst_width, layer_width, box_info_list, connections):
        """Draw a measurement instruction"""
        clbit = inst.clbits[0] if len(inst.clbits) > 0 else 0
        self.measurement_info.append({'col': current_col + layer_width // 2, 'qubit': inst.qubits[0], 'clbit': clbit})
        symbol = self._get_gate_symbol(inst)
        box_info = self._draw_box(y_coords[0], symbol, current_col, inst_width, layer_width)
        self._add_box_info(box_info, y_coords[0], box_info_list)
        connections.append({'type': 'measurement', 'y': y_coords[0], 'x': current_col + layer_width // 2})

    def _draw_barrier_instruction(self, inst, y_coords, current_col, layer_width):
        """Draw a barrier instruction"""
        center = current_col + layer_width // 2
        for y in y_coords:
            self.grid[y][center] = self.config.barrier

    def _draw_gate_instruction(self, inst, y_coords, current_col, inst_width, layer_width, box_info_list, connections):
        """Draw a gate instruction - second level dispatch by gate name"""
        gate_name = inst.name.upper()

        # Second level dispatch: by gate name
        if gate_name in ["CX", "CNOT"]:
            self._draw_controlled_gate(inst, [inst.qubits[0]], inst.qubits[1], "X",
                                      current_col, inst_width, layer_width, box_info_list, connections)
        elif gate_name == "CZ":
            self._draw_two_qubit_symmetric_gate(inst, self._draw_control_dot,
                                               current_col, layer_width, connections)
        elif gate_name == "SWAP":
            self._draw_two_qubit_symmetric_gate(inst, self._draw_swap_symbol,
                                               current_col, layer_width, connections)
        elif gate_name in ["CCX", "TOFFOLI"]:
            self._draw_controlled_gate(inst, [inst.qubits[0], inst.qubits[1]], inst.qubits[2], "X",
                                      current_col, inst_width, layer_width, box_info_list, connections)
        elif len(inst.qubits) == 1:
            self._draw_single_qubit_gate(inst, y_coords, current_col, inst_width, layer_width, box_info_list)
        else:
            self._draw_multi_qubit_gate(inst, y_coords, current_col, inst_width, layer_width, box_info_list)

    def _draw_single_qubit_gate(self, inst, y_coords, current_col, inst_width, layer_width, box_info_list):
        """Draw single qubit gate"""
        symbol = self._get_gate_symbol(inst)
        box_info = self._draw_box(y_coords[0], symbol, current_col, inst_width, layer_width)
        self._add_box_info(box_info, y_coords[0], box_info_list)

    def _draw_multi_qubit_gate(self, inst, y_coords, current_col, inst_width, layer_width, box_info_list):
        """Draw generic multi-qubit gate"""
        symbol = self._get_gate_symbol(inst)
        box_info = self._draw_multi_qubit_box(y_coords, symbol, current_col, inst_width, layer_width)
        for y in y_coords:
            self._add_box_info(box_info, y, box_info_list)

    def draw(self):
        """Main drawing method"""
        # 1. Get layered circuit
        layered_array = self.circuit.layered_circuit()
        if layered_array.ndim < 2 or layered_array.shape[1] < 2:
            return "" # Return empty string for empty circuit

        qubit_indices = layered_array[:, 0]
        layers = layered_array[:, 1:]

        # Calculate label width
        max_label_width = 0
        if self.num_qubits > 0:
            max_label_width = len(f"q_{self.num_qubits - 1}: ")
        if self.num_clbits > 0:
            max_label_width = max(max_label_width, len(f"c: {self.num_clbits}/"))

        # 2. Calculate layout
        columns = []
        for col_idx in range(layers.shape[1]):
            column_gates = layers[:, col_idx]
            max_width = 0
            for gate in column_gates:
                if gate is not None:
                    max_width = max(max_width, self._get_gate_width(gate))
            if max_width == 0: max_width = 1
            columns.append({'width': max_width, 'gates': column_gates})

        circuit_width = sum(c['width'] for c in columns)
        total_width = max_label_width + circuit_width

        # 3. Initialize grid
        label_width = self._initialize_grid(total_width, qubit_indices)
        current_col = label_width

        # 4. Draw each layer
        for col_data in columns:
            self._draw_layer(col_data, current_col)
            current_col += col_data['width']

        # 5. Add measurement numbers
        if self.num_clbits > 0:
            for info in self.measurement_info:
                num_str = str(info['clbit']) # Using clbit as per original logic, can change to qubit
                start = info['col'] - len(num_str) // 2
                self.grid[self.measure_num_y][start:start+len(num_str)] = list(num_str)

        return self._grid_to_string()

    def _get_gate_symbol(self, inst):
        gate_name = inst.name
        gate_symbols = {
            "i": "I", "x": "X", "y": "Y", "z": "Z",
            "h": "H", "s": "S", "t": "T",
            "rx": "Rx", "ry": "Ry", "rz": "Rz",
            "p": "P", "u3": "U3", "u": "U",
            "cnot": "X", "cx": "X", "cz": "Z",
            "swap": "×", "ccx": "X", "toffoli": "X",
            "sdg": "S†", "tdg": "T†",
            "iswap": "iS",
            "rzz": "Rzz", "rxx": "Rxx", "ryy": "Ryy",
            "measure": "M"
        }
        symbol = gate_symbols.get(gate_name.lower(), gate_name.upper())

        # Handle parameterized gates by checking the .paras property
        if hasattr(inst, 'paras') and inst.paras:
            param_strings = []
            for p in inst.paras:
                val = p.eval()
                param_strings.append(f"{val:.2f}") # Format to 2 decimal places

            if param_strings:
                symbol = f"{symbol}({','.join(param_strings)})"

        return symbol

    def _grid_to_string(self):
        """Convert grid to string output"""
        lines = []
        for row in self.grid:
            lines.append("".join(row).rstrip())
        return "\n".join(lines)


def draw_circuit(circuit, unicode=True):
    """
    Draw quantum circuit as ASCII text (Qiskit style)
    """
    # The unicode argument is kept for API compatibility but is no longer used.
    drawer = CircuitTextDrawer(circuit)
    return drawer.draw()