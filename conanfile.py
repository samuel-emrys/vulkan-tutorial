from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
from conan.tools.build import check_min_cppstd
import os


class VulkanConan(ConanFile):
    name = "vulkan-tutorial"
    version = "0.1.0"
    license = "GNU GPL v3.0"
    author = "Samuel Dowling <samuel.dowling@protonmail.com>"
    url = "https://github.com/samuel-emrys/vulkan-tutorial"
    description = "Vulkan Tutorial"
    topics = ("gaming", "raytracing", "graphics")
    test_type = "explicit"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "shared": [True, False],
        "fPIC": [True, False],
    }
    default_options = {
        "shared": False,
        "fPIC": True,
    }
    exports_sources = (
        "src/*",
        "include/*",
        "LICENSE",
        "CMakeLists.txt",
    )

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        if self.options.shared:
            self.options.rm_safe("fPIC")

    def validate(self):
        if self.settings.compiler.cppstd:
            check_min_cppstd(self, "20")

    def build_requirements(self):
        self.tool_requires("cmake/[>=3.22.0]")

    def requirements(self):
        self.requires("vulkan-headers/1.3.239.0")
        self.requires("vulkan-loader/1.3.239.0")
        self.requires("vulkan-validationlayers/1.3.239.0")
        self.requires("glfw/3.3.8")
        self.requires("glm/cci.20230113")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
