from setuptools import setup, Extension, find_packages
from sys import executable, argv
from os.path import join, dirname, abspath

try:
   from numpy.distutils.ccompiler import CCompiler_compile
   import distutils.ccompiler
   distutils.ccompiler.CCompiler.compile = CCompiler_compile
except ImportError:
   print("Numpy not found, parallel compile not available")



maboss_version = '1.0.0b14'

maboss_sources = [
   "FinalStateSimulationEngine.cc", "MetaEngine.cc", "MaBEstEngine.cc", "EnsembleEngine.cc", 
   "Cumulator.cc", "ProbaDist.cc", "BooleanNetwork.cc", "BooleanGrammar.cc", "RunConfigGrammar.cc", 
   "Function.cc", "BuiltinFunctions.cc", "RunConfig.cc", "LogicalExprGen.cc", "Utils.cc"
]

maboss_module_sources = [
   'cmaboss/maboss_module.cpp', 
]
extra_compile_args = ['-std=c++11', '-DPYTHON_API', '-DSBML_COMPAT']

def getExtensionByMaxnodes(maxnodes=64):
   import numpy
   return Extension(
      "cmaboss.cmaboss" if maxnodes <= 64 else "cmaboss_%dn.cmaboss_%dn" % (maxnodes, maxnodes), 
      sources=maboss_module_sources + ["cmaboss/src/%s" % source for source in maboss_sources], 
      include_dirs=[numpy.get_include()],
      extra_compile_args=(extra_compile_args + (['-DMAXNODES=%d' % max(maxnodes, 64)])),
      libraries=["sbml"],
      language="c++"
   )

setup (name = 'cmaboss',
   version = maboss_version,
   author = "contact@vincent-noel.fr",
   description = """MaBoSS python bindings""",
   ext_modules = [
      getExtensionByMaxnodes(), 
      getExtensionByMaxnodes(128), 
      getExtensionByMaxnodes(256), 
      getExtensionByMaxnodes(512),
      getExtensionByMaxnodes(1024)
   ],
   install_requires = ["numpy"],
   packages=find_packages()
)
