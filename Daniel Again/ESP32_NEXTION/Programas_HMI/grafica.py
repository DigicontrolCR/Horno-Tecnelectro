import matplotlib.pyplot as plt

# Crear una figura con las dimensiones especificadas (450x200 píxeles)
fig = plt.figure(figsize=(4.5, 2), dpi=100)

# Configurar el fondo negro
ax = fig.add_subplot(111)
ax.set_facecolor('black')
fig.patch.set_facecolor('black')

# Dibujar el eje X (borde inferior) y el eje Y (borde izquierdo)
plt.axhline(y=0, color='white', linewidth=1)  # Eje X en el borde inferior
plt.axvline(x=0, color='white', linewidth=1)  # Eje Y en el borde izquierdo

# Ajustar los límites de los ejes para que el eje Y vaya de 0 a 150
ax.set_xlim(0, 10)
ax.set_ylim(15, 150)  # Ajustamos el inicio del eje Y en 15

# Establecer las divisiones del eje Y en intervalos de 15, comenzando desde 15
ax.set_yticks(range(15, 151, 15))

# Establecer el color y formato de las etiquetas
ax.tick_params(axis='y', colors='white', labelsize=6)  # Reducir tamaño de letra en el eje Y

# Eliminar las marcas del eje X
ax.tick_params(axis='x', which='both', bottom=False, top=False, labelbottom=False)

# Eliminar los bordes superiores y derechos
ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
ax.spines['left'].set_visible(False)
ax.spines['bottom'].set_visible(False)

# Ajustar el espacio para que los ejes queden exactamente en los bordes
plt.subplots_adjust(left=0, right=1, top=1, bottom=0)

# Agregar las etiquetas en la parte superior derecha con los colores deseados
ax.text(10, 150, 'Temperatura (°C)', color='red', fontsize=8, verticalalignment='top', horizontalalignment='right')
ax.text(10, 142, 'Corriente (A)', color='yellow', fontsize=8, verticalalignment='top', horizontalalignment='right')

# Dibujar las líneas punteadas a lo largo del eje X para cada valor en el eje Y
for y in range(15, 151, 15):
    ax.plot([0, 10], [y, y], color='white', linestyle=':', linewidth=0.5)  # Línea punteada

# Guardar la imagen y mostrarla
plt.savefig("ejes_con_lineas_punteadas.png", bbox_inches='tight', pad_inches=0)
plt.show()
