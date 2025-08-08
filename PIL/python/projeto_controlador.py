from scipy.optimize import fsolve
import control
import numpy as np
from matplotlib import pyplot as plt

# Parâmetros do Boost
fsw = 10000
Vin = 12
Vout = 20    # Tensão desejada
L = 1e-3
C = 10e-6
R = 10
vcp = 1
Tp = 1 / (2*np.pi*fsw*0.1)

PM = 60   # graus
FC = 600  # Hz

# Cálculo do ponto de operação
D = 1 - (Vin / Vout)  # razão cíclica no ponto de operação

# Função de transferência do Boost (modo contínuo, tensão -> tensão)
Gp = control.tf([Vin / (1-D)],
                [L*C, L/(R*(1-D)**2), 1])
Gpwm = control.tf([1],[vcp])
Gs = control.tf([1],[1])

# Controlador PI + avanço
Gc = lambda k,T: control.series(
    control.tf([k],[1]),
    control.tf([T, 1], [T, 0]),
    control.tf([1], [Tp, 1])
)

Gol = lambda k,T: control.series(Gc(k,T), Gpwm, Gp, Gs)

def func(x):
    k, T = x
    if k < 0 or T < 0:
        return [1e10, 1e10]
    gm, pm, wcg, wcp = control.margin(Gol(k,T))
    return [pm - PM, wcp - 2*np.pi*FC]

# Resolver para k e T
k, T = fsolve(func, [0.001, 0.001], factor=10, maxfev=10000)

# Resposta em frequência
omega = np.logspace(0, 5, 1000)
control.bode(Gol(k,T), omega, Hz=True, dB=True, deg=True)
gm, pm, wcg, wcp = control.margin(Gol(k,T))
print(f'Margem de fase: {pm:.2f}º Frequência de cruzamento: {wcp/(2*np.pi):.2f}')
print(f'Parâmetros: k = {k} | T = {T}')
plt.show()

# Discretização do controlador
fs = fsw
controller = control.sample_system(Gc(k,T), 1/fs, method='bilinear')
print(controller)

# Geração dos coeficientes para implementação
output = ''
order = max(len(controller.num[0][0]), len(controller.den[0][0]))
for i in range(order):
    output += f"#define B{i} {controller.num[0][0][i]/controller.den[0][0][0]:.6f}\r\n"
for i in range(1, order):
    output += f"#define A{i} {controller.den[0][0][i]/controller.den[0][0][0]:.6f}\r\n"
print(output)
