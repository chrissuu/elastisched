from setuptools import setup, find_packages

setup(
    name="elastisched",
    version="0.1.0",
    description="Elastic scheduling package",
    author="Chris Su",
    author_email="chrisssu19@gmail.com",
    packages=find_packages(where="src"),
    package_dir={"": "src"},
    python_requires=">=3.11"
)