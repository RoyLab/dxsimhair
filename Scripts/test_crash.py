import crash_on_ipy

def func():
    i = 0
    raise KeyError(1)

func()
