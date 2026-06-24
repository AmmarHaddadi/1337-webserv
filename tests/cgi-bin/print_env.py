import os

print("all env vars:")
for key, value in sorted(os.environ.items()):
    print(f"{key} = {value}")
print("end of all env vars\n")
