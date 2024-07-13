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

from typing import Tuple

import numpy as np
from scipy.linalg import expm

# Helper Functions

I = np.identity(2)
X = np.array([[0, 1], [1, 0]])
Y = np.array([[0, -1j], [1j, 0]])
Z = np.array([[1, 0], [0, -1]])


def decompose_one_qubit_product(
        U: np.ndarray, validate_input: bool = True, atol: float = 1e-8, rtol: float = 1e-5
):
    """
    Decompose a 4x4 unitary matrix to two 2x2 unitary matrices.
    Args:
        U (np.ndarray): input 4x4 unitary matrix to decompose.
        validate_input (bool): if check input.
    Returns:
        phase (float): global phase.
        U1 (np.ndarray): decomposed unitary matrix U1.
        U2 (np.ndarray): decomposed unitary matrix U2.
        atol (float): absolute tolerance of loss.
        rtol (float): relative tolerance of loss.
    Raises:
        AssertionError: if the input is not a 4x4 unitary or
        cannot be decomposed.
    """

    """if validate_input:
        assert np.allclose(
            makhlin_invariants(U, atol=atol, rtol=rtol), (1, 0, 3), atol=atol, rtol=rtol
        )"""

    # find the maximum value's index of U
    i, j = np.unravel_index(np.argmax(U, axis=None), U.shape)

    def u1_set(i):
        return (1, 3) if i % 2 else (0, 2)

    def u2_set(i):
        return (0, 1) if i < 2 else (2, 3)

    # get the submatrix
    u1 = U[np.ix_(u1_set(i), u1_set(j))]
    u2 = U[np.ix_(u2_set(i), u2_set(j))]

    u1 = to_su(u1)
    u2 = to_su(u2)

    phase = U[i, j] / (u1[i // 2, j // 2] * u2[i % 2, j % 2])

    return phase, u1, u2


def to_su(u: np.ndarray) -> np.ndarray:
    """
    Given a unitary in U(N), return the
    unitary in SU(N).
    Args:
        u (np.ndarray): The unitary in U(N).
    Returns:
        np.ndarray: The unitary in SU(N)
    """
    # turn the determinent to 1
    return u * complex(np.linalg.det(u)) ** (-1 / np.shape(u)[0])


def weyl_chamber(c):
    """Bring coordinates vector into the Weyl chamber"""

    # Step 0: work in terms of multiple of pi
    c /= np.pi

    # Step 1: Bring everything into [0, 1)
    c -= np.floor(c)

    # Step 2: Sort c1 >= c2 >= c3
    c = np.sort(c)[::-1]

    # Step 3: if c1 + c2 >= 1, transform (c1, c2, c3) -> (1-c2, 1-c1, c3)
    if c[0] + c[1] >= 1:
        c = np.sort(np.array([1 - c[1], 1 - c[0], c[2]]))[::-1]

    # Step 4: if c3 = 0 and c1>1/2, transform (c1, c2, 0) -> (1-c1, c2, 0)
    if (c[0] > 1 / 2) and np.isclose(c[2], 0):
        c = np.array([1 - c[0], c[1], 0])

    # Step 5: Turn it back into radians
    c *= np.pi

    return c


def KAK_decomposition(
        U: np.ndarray,
        rounding: int = 19
) -> Tuple[float, np.ndarray, np.ndarray, float, np.ndarray, np.ndarray, float,
           float, float]:
    """
    Decomposes a 2-qubit unitary matrix into the product of three matrices:
    KAK = L @ CAN(theta_vec) @ R where L and R are two-qubit local unitaries, 
    CAN is a 3-parameter canonical matrix, and theta_vec is a vector of 3 angles.

    Args:
        U (np.ndarray): 2-qubit unitary matrix
        rounding (int): Number of decimal places to round intermediate 
        matrices to (default 14)

    Returns:
        Tuple of 9 values:
            - phase1 (float): Global phase factor for left local unitary L
            - L1 (np.ndarray): Top 2x2 matrix of left local unitary L
            - L2 (np.ndarray): Bottom 2x2 matrix of left local unitary L
            - phase2 (float): Global phase factor for right local unitary R
            - R1 (np.ndarray): Top 2x2 matrix of right local unitary R
            - R2 (np.ndarray): Bottom 2x2 matrix of right local unitary R
            - c0 (float): XX canonical parameter in the Weyl chamber
            - c1 (float): YY canonical parameter in the Weyl chamber
            - c2 (float): ZZ canonical parameter in the Weyl chamber
    """

    # 0. Map U(4) to SU(4) (and phase)
    global_phase = np.linalg.det(U) ** 0.25
    U /= global_phase

    assert np.isclose(np.linalg.det(U), 1), "Determinant of U is not 1"

    # 1. Unconjugate U into the magic basis
    B = 1 / np.sqrt(2) * np.array([[1, 0, 0, 1j], [0, 1j, 1, 0],
                                   [0, 1j, -1, 0], [1, 0, 0, -1j]])  # Magic Basis
    U_prime = np.conj(B).T @ U @ B

    Theta = lambda U: np.conj(U)
    # Isolating the maximal torus
    M_squared = Theta(np.conj(U_prime).T) @ U_prime

    if rounding is not None:
        M_squared = np.round(M_squared, rounding)  # For numerical stability

    ## 2. Diagonalizing M^2
    D, P = np.linalg.eig(M_squared)

    ## Check and correct for det(P) = -1
    if np.isclose(np.linalg.det(P), -1):
        P[:, 0] *= -1  # Multiply the first eigenvector by -1

    # 3. Extracting K2
    K2 = np.conj(P).T

    assert np.allclose(K2 @ K2.T, np.identity(4)), "K2 is not orthogonal"
    assert np.isclose(np.linalg.det(K2), 1), "Determinant of K2 is not 1"

    # 4. Extracting A
    A = np.sqrt(D)

    ## Check and correct for det(A) = -1
    if np.isclose(np.prod(A), -1):
        A[0] *= -1  # Multiply the first eigenvalue by -1

    A = np.diag(A)  # Turn the list of eigenvalues into a diagonal matrix

    assert np.isclose(np.linalg.det(A), 1), "Determinant of A is not 1"

    # 5. Extracting K1
    K1 = U_prime @ np.conj(K2).T @ np.conj(A).T

    assert np.allclose(K1 @ K1.T, np.identity(4)), "K1 is not orthogonal"
    assert np.isclose(np.linalg.det(K1), 1), "Determinant of K1 is not 1"

    # 6. Extracting Local Gates
    L = B @ K1 @ np.conj(B).T  # Left Local Product
    R = B @ K2 @ np.conj(B).T  # Right Local Product

    phase1, L1, L2 = decompose_one_qubit_product(L)  # L1 (top), L2(bottom)
    phase2, R1, R2 = decompose_one_qubit_product(R)  # R1 (top), R2(bottom)

    # 7. Extracting the Canonical Parameters
    C = np.array([[1, 1, 1], [-1, 1, -1], [1, -1, -1]])  # Coefficient Matrix

    theta_vec = np.angle(np.diag(A))[:3]  # theta vector
    a0, a1, a2 = np.linalg.inv(C) @ theta_vec  # Computing the "a"-vector

    CAN = lambda c0, c1, c2: expm(1j / 2 * (c0 * np.kron(X, X) + c1 * np.kron(Y, Y) + c2 * np.kron(Z, Z)))
    # 8. Unpack Parameters and Put into Weyl chamber
    c0, c1, c2 = 2 * a1, -2 * a0, 2 * a2
    # print(c0, c1, c2)
    # print((phase1 * np.kron(L1, L2)) @ CAN(c0, c1, c2)
    #                   @ (phase2 * np.kron(R1, R2)))
    # c0, c1, c2 = weyl_chamber(np.array([2 * a1, -2 * a0, 2 * a2]))
    # print(c0, c1, c2)
    # print((phase1 * np.kron(L1, L2)) @ CAN(c0, c1, c2)
    #                   @ (phase2 * np.kron(R1, R2)))
    assert np.allclose(U, (phase1 * np.kron(L1, L2)) @ CAN(c0, c1, c2)
                       @ (phase2 * np.kron(R1, R2))), "U does not equal KAK"

    assert np.allclose(U, phase1 * phase2 * (np.kron(L1, L2)) @ CAN(c0, c1, c2)
                       @ (np.kron(R1, R2))), "U does not equal KAK"
    # print(np.allclose(U, phase1 *phase2 *( np.kron(L1, L2)) @ CAN(c0, c1, c2)
    #                   @ ( np.kron(R1, R2))))
    return phase1, L1, L2, phase2, R1, R2, c0, c1, c2

# U = unitary_group.rvs(4, random_state = 1)
# print(U)
# print(KAK_decomposition(U))
# print(np.kron(Y,Y))
