OPENQASM 2.0;
include "qelib1.inc";
qreg q[6];
creg meas[6];
ccx q[0],q[1],q[3];
ccx q[0],q[1],q[3];
u1(0.1) q[0];
u2(0.1, 0.2) q[1];
// u3(0.1, 0.2, 0.3) q[2];
u1(0.2+theta) q[3];
u2(3*(0.1+pi)-2, 0.2*pi) q[3];
CX q[0], q[1];
CX q[0], q[1], q[3];

gate cphase(lambda) a,b,b {
    u1(lambda/2) a;
    CX a,b[0];
    u1(-lambda/2) b;
    CX a,b;
    // u1(lambda/2) b;
    CX a,b;
    CX a,b;
    U(0,pi,pi/2) a;
}