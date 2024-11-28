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
for num_procs in df['num_procs'].unique():
    data = df[df['num_procs'] == num_procs]
    plt.plot(data['array_size'], data['time'], marker='o', label=f'{num_procs} processors')

plt.xlabel('Array Size')
plt.ylabel('Time (seconds)')
plt.title('QuickSort Performance Analysis')
plt.legend()
plt.grid(True)
plt.xscale('log')
plt.yscale('log')

# Guardar la gráfica
plt.savefig('quicksort_performance.png')

# Calcular speedup
base_data = df[df['num_procs'] == 1]
base_times = base_data.set_index('array_size')['time']
speedup_data = []

for _, row in df.iterrows():
    # Find the closest array size in base_times
    array_size = row['array_size']
    closest_size = base_times.index[abs(base_times.index - array_size).argmin()]
    base_time = base_times[closest_size]
    speedup = base_time / row['time']
    speedup_data.append({
        'array_size': row['array_size'],
        'num_procs': row['num_procs'],
        'speedup': speedup
    })

speedup_df = pd.DataFrame(speedup_data)

# Crear gráfica de speedup
plt.figure(figsize=(10, 6))
for size in speedup_df['array_size'].unique():
    data = speedup_df[speedup_df['array_size'] == size]
    plt.plot(data['num_procs'], data['speedup'], marker='o', label=f'Size {size}')

plt.plot([1, max(df['num_procs'])], [1, max(df['num_procs'])], 'k--', label='Ideal speedup')
plt.xlabel('Number of Processors')
plt.ylabel('Speedup')
plt.title('QuickSort Speedup Analysis')
plt.legend()
plt.grid(True)

# Guardar la gráfica de speedup
plt.savefig('quicksort_speedup.png')

# Calcular y mostrar estadísticas
print("\nPerformance Statistics:")
print("\nAverage time by array size:")
print(df.groupby('array_size')['time'].mean())
print("\nAverage speedup by number of processors:")
print(speedup_df.groupby('num_procs')['speedup'].mean())