# -*- python -*-

env = Environment(CCFLAGS = ['-Wall'])

env.Program('a3dcontrol', 'src/a3dcontrol.c')

SConscript(['tools/SConscript',
            'examples/SConscript',
            ])
