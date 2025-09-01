import argparse
import os
from collections import defaultdict

def run():
    # Parse the folder with all dump files
    parser = argparse.ArgumentParser(description="Merge all dump files into one")
    parser.add_argument("folder", type=str, help="Folder with all dump files")
    args = parser.parse_args()

    # Get all files in the folder
    files = os.listdir(args.folder)
    files = [f for f in files if f.endswith(".dump") and f != "merged.dump"]
    # Read each file and gather all unique lines across all files
    lines = set()
    for file in files:
        with open(os.path.join(args.folder, file), "r") as f:
            lines.update(f.readlines())

    # Dictionary that starts with an empty list for each key
    merged_dict = defaultdict(list)
    for l in lines:
        index_call = l.split("@")
        merged_dict[index_call[1]].append(index_call[0])

    # Write the dictionary to a new file
    with open(os.path.join(args.folder, "merged.dump"), "w") as f:
        for call, indices in merged_dict.items():
            f.write(",".join(indices) + "@" + call)


if __name__ == "__main__":
    run()