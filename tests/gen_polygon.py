#!/usr/bin/env python3
import sys
import math

if len(sys.argv) != 2:
    print(f"uso: {sys.argv[0]} N", file=sys.stderr)
    print("gera um poligono convexo regular com 2*N vertices, em ordem anti-horaria", file=sys.stderr)
    sys.exit(1)

n_half = int(sys.argv[1])
n = n_half * 2
R = 4000.0
CX, CY = 5000.0, 5000.0

print(n_half)
for k in range(n):
    angle = 2 * math.pi * k / n
    x = CX + R * math.cos(angle)
    y = CY + R * math.sin(angle)
    print(f"{x:.4f} {y:.4f}")
