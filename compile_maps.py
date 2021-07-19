import argparse
import subprocess
import os
import sys

q3map2_executable = "q3map2_urt"

def main():
	parser= argparse.ArgumentParser(description= 'Maps compilation script.')
	parser.add_argument("--maps-dir", help= "Directory with maps", type=str)
	parser.add_argument("--baseq3-dir", help= "Game base directory", type=str)

	args= parser.parse_args()

	maps_dir = os.path.abspath(args.maps_dir)
	baseq3_dir = os.path.abspath(args.baseq3_dir)

	working_directory = os.path.split(os.path.abspath(q3map2_executable))[0]

	for file_name in os.listdir(maps_dir):
		if not file_name.endswith("map"):
			continue

		map_path = os.path.join(maps_dir, file_name)

		# Compile map itself
		q3map2_args = [q3map2_executable, "-v", "-game", "quake3", "-fs_basepath", baseq3_dir, "-meta", map_path]
		print("Executing " + str(q3map2_args))
		subprocess.run(q3map2_args, cwd= working_directory)

		# make light
		q3map2_args = [q3map2_executable, "-v", "-game", "quake3", "-fs_basepath", baseq3_dir, "-light", "-point", "0.3", "-bounce", "0", map_path]
		print("Executing " + str(q3map2_args))
		subprocess.run(q3map2_args, cwd= working_directory)


if __name__ == "__main__":
	sys.exit(main())
