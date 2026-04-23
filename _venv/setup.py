import os
import sys
import subprocess

VENV_DIR = os.path.dirname(os.path.abspath(__file__))  # Path to venv folder
PYTHON_EXEC = sys.executable  # Current Python interpreter

def create_venv():
	"""Create the virtual environment in the venv directory."""
	if not os.path.exists(os.path.join(VENV_DIR, "bin" if os.name != "nt" else "Scripts")):
		print("Creating virtual environment...")
		subprocess.run([PYTHON_EXEC, "-m", "venv", VENV_DIR], check=True)
	else:
		print("Virtual environment already exists.")

def install_requirements():
	"""Install dependencies from requirements.txt if it exists."""
	req_file = os.path.join(VENV_DIR, "requirements.txt")
	if os.path.exists(req_file):
		print("Installing dependencies from requirements.txt...")
		pip_path = os.path.join(VENV_DIR, "bin" if os.name != "nt" else "Scripts", "pip")
		subprocess.run([pip_path, "install", "-r", req_file], check=True)
	else:
		print("No requirements.txt found. Skipping package installation.")

if __name__ == "__main__":
	create_venv()
	install_requirements()
	print("Setup complete! To activate the environment, run:")
	print(f"  source {VENV_DIR}/bin/activate" if os.name != "nt" else f"  {VENV_DIR}\\Scripts\\activate")
