import matplotlib
import matplotlib.pyplot as plt
from matplotlib import cm
from matplotlib.ticker import LinearLocator
import numpy as np

fig, ax = plt.subplots(subplot_kw={"projection": "3d"})
matplotlib.rcParams['legend.fontsize'] = 8
matplotlib.rcParams['font.size'] = 12
# Make data.
# masking order
X = np.arange(1, 13, 1)
# memory refernces
Y = np.arange(1, 101, 1)
X, Y = np.meshgrid(X, Y)
R = (0.75*X+0.25*X**2) * (Y)
Z = R
print(matplotlib.rcParams.keys())
# Plot the surface.
surf1 = ax.plot_surface(X, Y, 20.351*Z, cmap=cm.binary,
                       linewidth=5, antialiased=True, alpha=0.7, label="x64 RF")
surf2 = ax.plot_surface(X, Y, 10.613*Z, cmap=cm.Reds,
                       linewidth=10, antialiased=True, alpha=0.7, label="XMMs")
surf3 = ax.plot_surface(X, Y, 8.338*Z, cmap=cm.Greens,
                       linewidth=20, antialiased=True, alpha=0.7, label="YMMs")

surf1._edgecolors2d = surf1._edgecolor3d
surf1._facecolors2d = surf1._facecolor3d
surf2._edgecolors2d = surf2._edgecolor3d
surf2._facecolors2d = surf2._facecolor3d
surf3._edgecolors2d = surf3._edgecolor3d
surf3._facecolors2d = surf3._facecolor3d
# Customize the z axis.
#ax.set_zlim(0, 100)
#ax.zaxis.set_major_locator(LinearLocator(10))
# A StrMethodFormatter is used automatically
#ax.zaxis.set_major_formatter('{x:.02f}')

# Add a color bar which maps values to colors.
#fig.colorbar(surf, shrink=0.3, aspect=3)

# ...
ax.legend()
ax.set(
    xlabel="masking order",
    ylabel="#state reads/writes",
    zlabel="time[us]",
    title="masking and SIMD effects on comp. time",
    )

plt.show()
