import argparse
import subprocess
import os
import sys

q3map_executable = "q3map"

def main():
	parser= argparse.ArgumentParser(description= 'Maps compilation script.')
	parser.add_argument("--maps-dir", help= "Directory with maps", type=str)

	args= parser.parse_args()

	maps_dir = os.path.abspath(args.maps_dir)

	for file_name in os.listdir(maps_dir):
		if not file_name.endswith("map"):
			continue

		#if file_name != "e1m1.map":
		#	continue

		map_path = os.path.join(maps_dir, file_name)

		# Compile map itself
		q3map_args = [q3map_executable, map_path]
		print("Executing " + str(q3map_args))
		subprocess.run(q3map_args)

		# make light
		q3map_args = [q3map_executable, "-light", "-point", "0.3", map_path]
		print("Executing " + str(q3map_args))
		subprocess.run(q3map_args)

		# make vis
		q3map_args = [q3map_executable, "-vis", map_path]
		print("Executing " + str(q3map_args))
		subprocess.run(q3map_args)


if __name__ == "__main__":
	sys.exit(main())
