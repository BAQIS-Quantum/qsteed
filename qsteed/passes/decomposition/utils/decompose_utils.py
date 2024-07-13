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

import re

import numpy as np
import scipy.linalg

from qsteed.passes.decomposition.utils import matrix_utils as mu


def demultiplexing(U1, U2):
    """
    U = [U1  0] = [V   0][D  0][W  0]
        [0  U2] = [0   V][0 D*][0  W]
    """
    assert mu.is_unitary(U1)
    assert mu.is_unitary(U2)

    U2d = np.conjugate(U2).T
    x = U1 @ U2d

    # print(x)
    # d, V = np.linalg.eig(U1@U2d)
    D2, V = scipy.linalg.schur(x)
    assert mu.is_diagonal(D2)
    assert mu.is_approx(V @ D2 @ np.conjugate(V).T, U1 @ U2d)

    d_tmp = np.sqrt(np.diag(D2))
    D = np.diag(d_tmp)

    assert mu.is_approx(D @ D, D2)
    Vd = np.conjugate(V).T

    W = D @ Vd @ U2

    assert mu.is_approx(U1, V @ D @ W)
    assert mu.is_approx(U2, V @ (np.conjugate(D).T) @ W)

    ZM = np.zeros_like(W)
    VV = mu.stack_matrices(V, ZM, ZM, V)
    DD = mu.stack_matrices(D, ZM, ZM, np.conjugate(D).T)
    WW = mu.stack_matrices(W, ZM, ZM, W)
    UU = mu.stack_matrices(U1, ZM, ZM, U2)

    # print(isApprox(VV@DD@WW, UU))
    assert mu.is_approx(VV @ DD @ WW, UU)
    assert mu.is_unitary(V)
    return V, D, W


def graycode(n):
    gray_str = []
    for i in range(0, 1 << n):
        gray = i ^ (i >> 1)
        gray_str.append("{0:0{1}b}".format(gray, n))

    return gray_str


def get_multi_control_index(n):
    gray_str = graycode(n)
    size = len(gray_str)

    index_list = []
    for i in range(size):
        str1 = gray_str[i]
        str2 = gray_str[(i + 1) % size]

        tmp = [k for k in range(len(str1)) if str1[k] != str2[k]]
        assert len(tmp) == 1
        index_list.append(tmp[0])

    return index_list


def bitParity(i):
    if i < 2 << 16:
        i = (i >> 16) ^ i
        i = (i >> 16) ^ i
        i = (i >> 8) ^ i
        i = (i >> 4) ^ i
        i = (i >> 2) ^ i
        i = (i >> 1) ^ i
        return i % 2
    else:
        print("Bit parity number too big!")


def genMk2(nqubits):
    genMk_lookuptable = []
    for n in range(1, nqubits + 1):
        size = 2 ** n
        Mk = np.zeros((size, size))
        for i in range(size):
            for j in range(size):
                Mk[i, j] = (-1) ** bitParity(i & j ^ (j >> 1))

        genMk_lookuptable.append(Mk)

    return genMk_lookuptable


def bin2gray(num):
    return num ^ int((num >> 1))


def genMk(k):
    Mk = np.zeros((2 ** k, 2 ** k))
    for i in range(2 ** k):
        for j in range(2 ** k):
            p = i & bin2gray(j)
            strbin = "{0:b}".format(p)
            tmp = [m.start() for m in re.finditer('1', strbin)]
            Mk[i, j] = (-1) ** len(tmp)

    return Mk


def genMk_table(nqubits):
    genMk_lookuptable = []
    for n in range(1, nqubits + 1):
        tmp = genMk(n)
        genMk_lookuptable.append(tmp)

    return genMk_lookuptable
