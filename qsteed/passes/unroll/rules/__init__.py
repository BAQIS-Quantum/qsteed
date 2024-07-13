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

from __future__ import annotations

from qsteed.passes.unroll.rules.cnot2cp import CNOTToCP
from qsteed.passes.unroll.rules.cnot2cz import CNOTToCZ
from qsteed.passes.unroll.rules.cnot2iswap import CNOTToISWAP
from qsteed.passes.unroll.rules.cp2cnot import CPToCNOT
from qsteed.passes.unroll.rules.crx2cnot import CRXToCNOT
from qsteed.passes.unroll.rules.cry2cnot import CRYToCNOT
from qsteed.passes.unroll.rules.crz2cnot import CRZToCNOT
from qsteed.passes.unroll.rules.cs2cnot import CSToCNOT
from qsteed.passes.unroll.rules.ct2cnot import CTToCNOT
from qsteed.passes.unroll.rules.cy2cnot import CYToCNOT
from qsteed.passes.unroll.rules.cz2cnot import CZToCNOT
from qsteed.passes.unroll.rules.fredkin2toffoli import FredkinToToffoli
from qsteed.passes.unroll.rules.h2ryrz import HToRYRZ
from qsteed.passes.unroll.rules.iswap2cnot import ISWAPToCNOT
from qsteed.passes.unroll.rules.mcrx2cnot import MCRXToCNOT
from qsteed.passes.unroll.rules.mcry2cnot import MCRYToCNOT
from qsteed.passes.unroll.rules.mcrz2cnot import MCRZToCNOT
from qsteed.passes.unroll.rules.mcx2cnot import MCXToCNOT
from qsteed.passes.unroll.rules.mcy2cnot import MCYToCNOT
from qsteed.passes.unroll.rules.mcz2cnot import MCZToCNOT
from qsteed.passes.unroll.rules.phase2rz import PhaseToRZ
from qsteed.passes.unroll.rules.rxx2cnot import RXXToCNOT
from qsteed.passes.unroll.rules.ryy2cnot import RYYToCNOT
from qsteed.passes.unroll.rules.rzz2cnot import RZZToCNOT
from qsteed.passes.unroll.rules.s2rz import SToRZ
from qsteed.passes.unroll.rules.sdg2rz import SdgToRZ
from qsteed.passes.unroll.rules.sw2ryrz import SWToRYRZ
from qsteed.passes.unroll.rules.swap2cnot import SwapToCNOT
from qsteed.passes.unroll.rules.swdg2ryrz import SWdgToRYRZ
from qsteed.passes.unroll.rules.sx2rx import SXToRX
from qsteed.passes.unroll.rules.sxdg2rx import SXdgToRX
from qsteed.passes.unroll.rules.sy2ry import SYToRY
from qsteed.passes.unroll.rules.sydg2ry import SYdgToRY
from qsteed.passes.unroll.rules.t2rz import TToRZ
from qsteed.passes.unroll.rules.tdg2rz import TdgToRZ
from qsteed.passes.unroll.rules.toffoli2cnot import ToffoliToCNOT
from qsteed.passes.unroll.rules.toffoli2cnot import ToffoliToCNOT8
# from quabc.passes.unroll.rules.mcu2cnot import ControlledUToCNOT
from qsteed.passes.unroll.rules.u3decompose import U3Decompose
from qsteed.passes.unroll.rules.w2ryrz import WToRYRZ
from qsteed.passes.unroll.rules.x2rx import XToRX
from qsteed.passes.unroll.rules.y2ry import YToRY
from qsteed.passes.unroll.rules.z2rz import ZToRZ

__all__ = [
    'CNOTToCP',
    'CNOTToCZ',
    'CNOTToISWAP',
    'CPToCNOT',
    'CSToCNOT',
    'CTToCNOT',
    'CYToCNOT',
    'CZToCNOT',
    'FredkinToToffoli',
    'HToRYRZ',
    'ISWAPToCNOT',
    'PhaseToRZ',
    'RXXToCNOT',
    'RYYToCNOT',
    'RZZToCNOT',
    'SToRZ',
    'SdgToRZ',
    'SWToRYRZ',
    'SWdgToRYRZ',
    'SwapToCNOT',
    'SXToRX',
    'SXdgToRX',
    'SYToRY',
    'SYdgToRY',
    'TToRZ',
    'TdgToRZ',
    'ToffoliToCNOT',
    'ToffoliToCNOT8',
    'WToRYRZ',
    'XToRX',
    'YToRY',
    'ZToRZ',
    'CRXToCNOT',
    'CRYToCNOT',
    'CRZToCNOT',
    'MCXToCNOT',
    'MCYToCNOT',
    'MCZToCNOT',
    'MCRXToCNOT',
    'MCRYToCNOT',
    'MCRZToCNOT',
    'U3Decompose',
    # 'ControlledUToCNOT',
]
