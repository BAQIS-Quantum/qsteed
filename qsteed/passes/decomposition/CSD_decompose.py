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

import numpy as np

from qsteed.passes.decomposition.utils import matrix_utils as mu


def thinCSD(q1, q2):
    p = q1.shape[0]
    # print("the size of q1/q2: {}".format(p))

    u1, c, v1 = np.linalg.svd(q1)
    v1d = np.conjugate(v1).T
    c = np.flip(c)

    cm = np.zeros((p, p), dtype=complex)
    np.fill_diagonal(cm, c)

    u1 = np.fliplr(u1)
    v1d = np.fliplr(v1d)

    q2 = q2 @ v1d

    # find the biggest index of c[k] <= 1/np.sqrt(2)
    k = 0
    for i in range(1, p):
        if c[i] <= 1 / np.sqrt(2):
            k = i

    k = k + 1
    # print("the k size: {}".format(k))

    u2, _ = np.linalg.qr(q2[:, 0:k], mode='complete')
    # u2, _= np.linalg.qr(q2, mode='complete')
    # print("the size of u2: {}".format(u2.shape))
    # print("the u2 matrix: {}".format(u2))
    s = np.conjugate(u2).T @ q2
    # print("the size of s: {}".format(s.shape))
    # print("the s matrix: {}".format(np.real(s)))

    if k < p:
        r2 = s[k:p, k:p]
        # print("the size of rs: {}".format(r2.shape))
        ut, ss, vt = np.linalg.svd(r2)
        vtd = np.conjugate(vt).T
        s[k:p, k:p] = np.diag(ss)
        cm[:, k:p] = cm[:, k:p] @ vtd
        u2[:, k:p] = u2[:, k:p] @ ut
        v1d[:, k:p] = v1d[:, k:p] @ vtd

        w = cm[k:p, k:p]
        z, r = np.linalg.qr(w, mode='complete')
        cm[k:p, k:p] = r
        u1[:, k:p] = u1[:, k:p] @ z

    for i in range(p):
        if np.real(cm[i, i]) < 0:
            cm[i, i] = -cm[i, i]
            u1[:, i] = -u1[:, i]
        if np.real(s[i, i]) < 0:
            s[i, i] = -s[i, i]
            u2[:, i] = -u2[:, i]

    return u1, u2, v1d, cm, s


def fatCSD(matrix):
    """
    U = [q1, U01] = [u1    ][c  s][v1  ]
        [q2, U11] = [    u2][-s c][   v2]
    """
    # print(matrix)
    U00, U01, U10, U11 = mu.split_matrix(matrix)

    L0, L1, R0, cc, ss = thinCSD(U00, U10)
    R0 = np.conjugate(R0).T
    ss = -ss

    # get the v2
    R1 = np.zeros_like(R0)
    p = R1.shape[0]
    for j in range(p):
        if np.abs(ss[j, j]) > np.abs(cc[j, j]):
            L0d = np.conjugate(L0).T
            tmp = L0d @ U01
            R1[j, :] = tmp[j, :] / ss[j, j]
        else:
            L1d = np.conjugate(L1).T
            tmp = L1d @ U11
            R1[j, :] = tmp[j, :] / cc[j, j]

    assert mu.is_approx(L0 @ cc @ R0, U00)
    assert mu.is_approx(-L1 @ ss @ R0, U10)
    assert mu.is_approx(L0 @ ss @ R1, U01)
    assert mu.is_approx(L1 @ cc @ R1, U11)

    zeros_m = np.zeros_like(L0)
    L = mu.stack_matrices(L0, zeros_m, zeros_m, L1)
    D = mu.stack_matrices(cc, ss, -ss, cc)
    R = mu.stack_matrices(R0, zeros_m, zeros_m, R1)
    assert mu.is_approx(matrix, L @ D @ R)

    return L0, L1, R0, R1, cc, ss  # L0, L1 is unitary
