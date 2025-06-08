from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout

class ByBitBotRecipe(ConanFile):
	name = "bybitbot"
	version = "0.1"
	settings = "os", "compiler", "build_type", "arch"

	options = {"shared": [True, False], "fPIC": [True, False]}
	default_options = {"shared": False, "fPIC": True}
	
	def requirements(self):
		self.requires("tgbot/1.8")
		self.requires("libcurl/8.12.1")
		self.requires("openssl/3.5.0")

	def build_requirements(self):
		self.tool_requires("cmake/3.23.5")
		self.test_requires("gtest/1.13.0")

	def generate(self):
		tc = CMakeToolchain(self, generator="Unix Makefiles")
		if self.settings.build_type == "Debug":
			tc.variables["CMAKE_C_FLAGS"] = "-fsanitize=address,undefined -fno-omit-frame-pointer"
			tc.variables["CMAKE_CXX_FLAGS"] = "-fsanitize=address,undefined -fno-omit-frame-pointer"
		tc.generate()
		deps = CMakeDeps(self)
		deps.generate()

	def build(self):
		cmake = CMake(self)
		cmake.configure()
		cmake.build()

	def layout(self):
		cmake_layout(self)