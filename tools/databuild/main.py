"""
    DataBuild command line entry point
"""
def main(args):

    import shaderlib
    print("shaderlib.test_func(6,1) =", shaderlib.test_func(6,1))

    print(args)
    return 0
