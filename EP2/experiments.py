#!/usr/bin/env python3

import pandas as pd
import matplotlib.pyplot as plt
import subprocess
import statistics as st
import os


d = [10, 100, 1000]
n = [5, 25, 250]

results = {}

for i in range(3):
    e1 = subprocess.call(["./test.sh", str(d[i]), str(n[1])])

    with open("time.out", 'r') as f:
        time = [float(r) - 1 for r in f.readlines()]
    with open("mem.out", 'r') as f:
        memory = [float(r) for r in f.readlines()]

    mean_memory = st.mean(memory)
    mean_time = st.mean(time)
    ci_memory = 1.96 * st.stdev(memory)
    ci_time = 1.96 * st.stdev(time)

    results[f"pista_{i}"] = (mean_memory, ci_memory,
                             mean_time, ci_time)

    e1 = subprocess.run(["./test.sh", str(d[1]), str(n[i])])

    with open("time.out", 'r') as f:
        time = [float(r) - 1 for r in f.readlines()]
    with open("mem.out", 'r') as f:
        memory = [float(r) for r in f.readlines()]

    mean_memory = st.mean(memory)
    mean_time = st.mean(time)
    ci_memory = 1.96 * st.stdev(memory)
    ci_time = 1.96 * st.stdev(time)

    results[f"corredor_{i}"] = (mean_memory, ci_memory,
                                mean_time, ci_time)

os.remove("time.out")
os.remove("mem.out")

pd_results = pd.DataFrame(data=results)
pd_results.to_csv("results.csv", index=False, header=True)
