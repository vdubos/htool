from __future__ import print_function
import sys
import ctypes
import numpy as np
import htool
import re
import matplotlib.pyplot as plt
import scipy.sparse.linalg as spla
from mpi4py import MPI
import math

comm = MPI.COMM_WORLD
size = comm.Get_size()
rank = comm.Get_rank()

scalar = ctypes.c_double

#  Data
n = 1000

# p: points in a square in the plane z=1
p=np.zeros((n,3))
size = int(math.sqrt(n)) # sqrt(n) must be an integer !!!
for j in range(0,size):
    for k in range(0,size):
        p[j+k*size,0] = j
        p[j+k*size,1] = k
        p[j,2] = 1


# My Matrix equivalent
@htool.getcoefFunc
def getcoef(i,j,r):
    r[0] = 1./(1.e-5+math.sqrt(np.vdot(p[i]-p[j],p[i]-p[j])))


@htool.getsubmatrixFunc
def getsubmatrix(I,J,n,m,r):
    for i in range(0,n):
        for j in range(0,m):
            r[j*n+i] = 1./(1.e-5+math.sqrt(np.vdot(p[I[i]]-p[J[j]],p[I[i]]-p[J[j]])))

# Htool parameters
htool.set_epsilon(1e-3)
htool.set_eta(100)
htool.set_minclustersize(10)

# Hmatrix
H = htool.HMatrixCreate(p, n, getcoef)
# H = htool.HMatrixCreatewithsubmat(p, n, getsubmatrix)
x = np.ones(n,dtype = scalar)
result = np.zeros(n,dtype = scalar)
htool.mvprod(H,x,result)

# def Hmv(v):
#     res = np.zeros(n,dtype = scalar)
#     htool.mvprod(H,v,res)
#     return res

# def iter(rk):
#     if (rank == 0):
#         print(np.linalg.norm(rk))

# HOp = spla.LinearOperator((n, n), dtype = scalar, matvec=Hmv)

# x = np.random.random(n)

# [xh,infoh] = spla.gmres(HOp, x, callback = iter)

# Output
htool.printinfos(H)
htool.display(H)

# Dense matrix
A = np.zeros((n,n),dtype = scalar)
for i in range(0,n):
    for j in range(0,n):
        A[i,j] = 1./(1.e-5+math.sqrt(np.vdot(p[i]-p[j],p[i]-p[j])))
# def Amv(v):
#     return A.dot(v)

# AOp = spla.LinearOperator((n, n), dtype = scalar, matvec=Amv)

# [xa,infoa] = spla.gmres(AOp, x, callback = iter)

if (rank == 0):
    print(np.linalg.norm(A.dot(x)-result)/np.linalg.norm(A.dot(x)))

#plt.scatter(p[:,0], p[:,1], c=xa, alpha=0.5)
#plt.gray()
#plt.show()