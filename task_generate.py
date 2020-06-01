from dataclasses import dataclass
from typing import List, Optional
from math import sqrt
import random
import numpy as np
from math import gcd as gcd2
from functools import reduce
import pickle

tick_rate = 200 # Hz (ticks/sec)

msecs = lambda n: n//(1000/tick_rate)
ticks_to_secs = lambda n: n/tick_rate
usecs_to_ticks = lambda usecs: (tick_rate * usecs) / 1e6

rng = np.random.default_rng()

def lcm2(a, b):
    return ((a // gcd2(a, b)) * b) if (a != 0 and b != 0) else 0


def lcm(args):
    return reduce(lcm2, args)


def gcd(args):
    return reduce(gcd2, args)


@dataclass
class Task:
    period: int
    relative_deadline: int
    wcet: int

    def is_tick_task(self, val=None):
        if (val is not None):
            self.tick_task = val
        else:
            if not hasattr(self, 'tick_task'):
                return False
            return self.tick_task

    def get_time_usage(self, t):
        return np.floor(((t + self.period - self.relative_deadline) / self.period)) * self.wcet

    def get_utilization(self):
        return self.wcet / self.period

    def to_c_task(self):
        return "{" + f".edf_period = {self.period}, .edf_relative_deadline = {self.relative_deadline}, .rm_priority = {self.rm_priority}, .wcet = {self.wcet}" + "}"


@dataclass
class TaskSet:
    tasks: List[Task]

    def __len__(self):
        return len(self.tasks)

    def get_demand(self, t) -> int:
        return sum(task.get_time_usage(t) for task in self.tasks)

    def get_utilization(self) -> float:
        return sum(task.get_utilization() for task in self.tasks)

    def get_hyperperiod(self) -> int:
        return lcm(task.period for task in self.tasks)

    def guarantee(self) -> bool:
        L_star = sum((task.period - task.relative_deadline) * task.get_utilization() for task in self.tasks)
        D_max = max(task.relative_deadline for task in self.tasks)
        hyperperiod = lcm(task.period for task in self.tasks)
        U = self.get_utilization()

        if (U > 1): return False
        if (L_star == 0): return True

        print(L_star, D_max, hyperperiod, U, sep=', ')

        L_star /= (1 - U)

        max_check = min(hyperperiod, max(D_max, L_star))

        for task in self.tasks:
            d_k = task.period
            while (d_k < max_check):
                if self.get_demand(d_k) >= d_k:
                    print(f"{self.get_demand(d_k)} >= {d_k}")
                    return False
                d_k += task.period

        return True

    def to_c_tasks(self, ts_name="task_set"):
        sorted_tasks = sorted(self.tasks, key=lambda t: t.period)
        sorted_tasks = [t for t in sorted_tasks if not t.is_tick_task()]
        for i, task in enumerate(sorted_tasks, start=2):
            task.rm_priority = i

        num_tasks = len(sorted_tasks)

        tasks = "{" + ', '.join(task.to_c_task() for task in sorted_tasks) + "}"
        hyperperiod = self.get_hyperperiod()

        return f"struct task_set {ts_name} = " + "{" + f".hyperperiod = {hyperperiod}, .num_tasks = {num_tasks}, .tasks = {tasks}" + "};"

    @staticmethod
    def get_random(num_tasks=16, max_u=1):
        tasks = [Task(1, usecs_to_ticks(250), usecs_to_ticks(50))]
        tasks[0].is_tick_task(True)
        U = tasks[0].get_utilization()
        while (len(tasks) == 1):
            for i in range(num_tasks):
                utilization = rng.uniform(0, max_u-U)
                if (utilization < 0): break
                period = int(rng.uniform(msecs(250), msecs(1000)))
                period = 10*(period // 10) # attempt to get some tractable hyperperiod
                wcet = int(period * utilization)
                if (wcet == 0): continue
                tasks.append(Task(period, int(period - max(0, (period - wcet)/3)), wcet))
                U += utilization
        return TaskSet(tasks)


def generate_tasks():
    sets = [TaskSet.get_random(16, rng.uniform(0.5, 0.95)) for i in range(10000)]
    succ_sets = []
    fail_sets = []

    for set in sets:
        if set.guarantee() and ticks_to_secs(set.get_hyperperiod()) < 120 and len(set.tasks) > 4:
            succ_sets.append(set)
        if not set.guarantee() and ticks_to_secs(set.get_hyperperiod()) < 120 and len(set.tasks) > 4:
            fail_sets.append(set)

    with open('task_sets.pickle', 'wb') as f:
        pickle.dump(succ_sets, f)

    return (succ_sets, fail_sets)
