# Dependencies.cmake - 外部依赖管理
# 包含主要的FetchContent依赖的声明和配置

include(FetchContent)
set(FETCHCONTENT_QUIET OFF)

function(fetch_with_mirrors name tag)
    set(multiValueArgs REPOSITORIES)
    cmake_parse_arguments(FETCH "" "" "${multiValueArgs}" ${ARGN})
    
    set(fetch_success FALSE)
    foreach(repo ${FETCH_REPOSITORIES})
        message(STATUS "正在尝试从 ${repo} 获取 ${name}...")
        
        FetchContent_Declare(
            ${name}
            GIT_REPOSITORY ${repo}
            GIT_TAG ${tag}
            GIT_PROGRESS TRUE
            GIT_SHALLOW TRUE
        )
        
        FetchContent_GetProperties(${name})
        string(TOLOWER ${name} name_lower)
        if(NOT ${name_lower}_POPULATED)
            message(STATUS "正在下载 ${name} 从 ${repo}...")
            FetchContent_Populate(${name})
            if(${name_lower}_POPULATED)
                set(fetch_success TRUE)
                message(STATUS "${name} 成功从 ${repo} 下载")
                break()
            else()
                message(WARNING "从 ${repo} 下载 ${name} 失败，尝试下一个镜像...")
                unset(${name}_DECLARED)
            endif()
        else()
            set(fetch_success TRUE)
            message(STATUS "${name} 已存在，跳过下载")
            break()
        endif()
    endforeach()
    
    if(NOT fetch_success)
        message(FATAL_ERROR "无法从任何镜像仓库下载 ${name}")
    endif()
endfunction()

# =============================================================================
# Eigen3 配置
# =============================================================================
message(STATUS "正在配置 Eigen3 依赖...")

set(EIGEN_BUILD_DOC OFF CACHE BOOL "Disable Eigen documentation")
set(EIGEN_BUILD_UNSUPPORTED OFF CACHE BOOL "Disable Eigen unsupported module")
set(EIGEN_BUILD_PKGCONFIG OFF CACHE BOOL "Disable Eigen pkgconfig")
set(EIGEN_BUILD_BENCHMARKS OFF CACHE BOOL "Disable Eigen benchmarks")

# Eigen3 镜像仓库列表
set(EIGEN_REPOS
    "https://gitlab.com/libeigen/eigen.git"                    # 官方仓库
    "https://github.com/eigenteam/eigen-git-mirror.git"       # GitHub镜像
    "https://gitee.com/mirrors/eigen.git"                     # Gitee镜像 (中国)
    "https://gitlab.freedesktop.org/mesa/eigen.git"           # FreeDesktop镜像
)

fetch_with_mirrors(Eigen3 "3.4.0" REPOSITORIES ${EIGEN_REPOS})
FetchContent_MakeAvailable(Eigen3)

# 修复 Eigen3 目标名称问题
if(NOT TARGET Eigen3::Eigen)
    if(TARGET eigen)
        add_library(Eigen3::Eigen ALIAS eigen)
        message(STATUS "创建 Eigen3::Eigen 别名指向 eigen")
    elseif(TARGET Eigen3)
        add_library(Eigen3::Eigen ALIAS Eigen3)
        message(STATUS "创建 Eigen3::Eigen 别名指向 Eigen3")
    else()
        # 手动创建接口库
        add_library(Eigen3::Eigen INTERFACE IMPORTED)
        set_target_properties(Eigen3::Eigen PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${eigen3_SOURCE_DIR}"
        )
        message(STATUS "手动创建 Eigen3::Eigen 接口库")
    endif()
endif()

message(STATUS "Eigen3 依赖配置完成")

# =============================================================================
# Autodiff 配置
# =============================================================================
message(STATUS "正在配置 autodiff 依赖...")

set(AUTODIFF_BUILD_TESTS OFF CACHE BOOL "Disable autodiff tests")
set(AUTODIFF_BUILD_PYTHON OFF CACHE BOOL "Disable autodiff Python bindings")
set(AUTODIFF_BUILD_EXAMPLES OFF CACHE BOOL "Disable autodiff examples")

set(AUTODIFF_REPOS
    "https://github.com/autodiff/autodiff.git"              # 官方仓库
    "https://gitee.com/mirrors/autodiff.git"                # Gitee镜像 (中国)
    "https://gitlab.com/autodiff/autodiff.git"              # GitLab镜像
)

fetch_with_mirrors(autodiff "v1.1.2" REPOSITORIES ${AUTODIFF_REPOS})
FetchContent_MakeAvailable(autodiff)

# 修复 autodiff 目标名称问题
if(NOT TARGET autodiff::autodiff)
    if(TARGET autodiff)
        add_library(autodiff::autodiff ALIAS autodiff)
        message(STATUS "创建 autodiff::autodiff 别名指向 autodiff")
    else()
        # 手动创建接口库
        add_library(autodiff::autodiff INTERFACE IMPORTED)
        set_target_properties(autodiff::autodiff PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${autodiff_SOURCE_DIR}"
        )
        message(STATUS "手动创建 autodiff::autodiff 接口库")
    endif()
endif()

message(STATUS "autodiff 依赖配置完成")

# =============================================================================
# FTXUI 配置
# =============================================================================
message(STATUS "正在配置 FTXUI 依赖...")

# set(FTXUI_BUILD_DOCS OFF CACHE BOOL "Disable FTXUI documentation")
# set(FTXUI_BUILD_EXAMPLES OFF CACHE BOOL "Disable FTXUI examples")
# set(FTXUI_BUILD_TESTS OFF CACHE BOOL "Disable FTXUI tests")
# set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "Disable FTXUI install")

# 使用标准的 FetchContent 方式
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
    GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

message(STATUS "FTXUI 依赖配置完成")