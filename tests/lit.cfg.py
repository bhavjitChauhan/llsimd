import lit.formats

config.name = 'llsimd Tests'
config.test_format = lit.formats.ShTest(True)
config.suffixes = ['.c']

config.substitutions.append(('%root', os.path.dirname(__file__) + '/..'))
