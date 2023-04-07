from distutils.core import setup, Extension


def main():
    setup(
        name="calculations",
        version="1.0.0",
        description="what's description?",
        ext_modules=[Extension("calculations", ["calculations.c"])],  # extra_compile_args=["-O2"]
    )


if __name__ == "__main__":
    main()