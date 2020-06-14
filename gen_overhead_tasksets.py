n_tasks = [32,64,128,256,512,1024]

for i in n_tasks:
    k = i//4
    print(k + k/2 + k/4 + k/8)
    for j in range(k):
        print(f"{{.edf_period = {2+i}, .edf_relative_deadline = {2+i}, .rm_priority = {0}, .wcet = {1}}},")
    for j in range(k//2):
        print(f"{{.edf_period = {2+i//2}, .edf_relative_deadline = {2+i//2}, .rm_priority = {0}, .wcet = {1}}},")
    for j in range(k//4):
        print(f"{{.edf_period = {2+i//4}, .edf_relative_deadline = {2+i//4}, .rm_priority = {0}, .wcet = {1}}},")
    for j in range(k//8):
        print(f"{{.edf_period = {2+i//8}, .edf_relative_deadline = {2+i//8}, .rm_priority = {0}, .wcet = {1}}},")

