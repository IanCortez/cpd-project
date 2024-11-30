"""
Comparar el tiempo de ejecucion y la velocidad los algoritmos proporcionados
"""

import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns

# Leer los datos del archivo
df = pd.read_csv('quicksort_performance.txt', names=['array_size', 'num_procs', 'time'])

# Crear gráfica de tiempo vs tamaño del array para diferentes números de procesadores
plt.figure(figsize=(10, 6))
for num_procs in sorted(df['num_procs'].unique()):
    data = df[df['num_procs'] == num_procs]
    mean_times = data.groupby('array_size')['time'].mean()
    plt.plot(mean_times.index, mean_times.values, marker='o', label=f'{num_procs} procesadores')

plt.xlabel('Tamaño del Array')
plt.ylabel('Tiempo (segundos)')
plt.title('Análisis de Rendimiento del Ranking Paralelo')
plt.legend()
plt.grid(True)
plt.xscale('log')
plt.yscale('log')

# Guardar la gráfica
plt.savefig('ranking_performance.png')
plt.close()

# Calcular speedup
base_times = df[df['num_procs'] == 1].groupby('array_size')['time'].mean()
speedup_data = []

for _, row in df.iterrows():
    array_size = row['array_size']
    if array_size in base_times.index:
        base_time = base_times[array_size]
        speedup = base_time / row['time']
        speedup_data.append({
            'array_size': array_size,
            'num_procs': row['num_procs'],
            'speedup': speedup
        })

speedup_df = pd.DataFrame(speedup_data)

# Crear gráfica de speedup
plt.figure(figsize=(10, 6))
for size in sorted(speedup_df['array_size'].unique()):
    data = speedup_df[speedup_df['array_size'] == size]
    mean_speedup = data.groupby('num_procs')['speedup'].mean()
    plt.plot(mean_speedup.index, mean_speedup.values, marker='o', label=f'Tamaño {size}')

max_procs = max(df['num_procs'])
plt.plot([1, max_procs], [1, max_procs], 'k--', label='Speedup ideal')
plt.xlabel('Número de Procesadores')
plt.ylabel('Speedup')
plt.title('Análisis de Speedup del Ranking Paralelo')
plt.legend()
plt.grid(True)

# Guardar la gráfica de speedup
plt.savefig('ranking_speedup.png')
plt.close()

# Calcular y mostrar estadísticas
print("\nEstadísticas de Rendimiento:")
print("\nTiempo promedio por tamaño de array:")
print(df.groupby('array_size')['time'].mean())
print("\nSpeedup promedio por número de procesadores:")
print(speedup_df.groupby('num_procs')['speedup'].mean())