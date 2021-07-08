# run script
# .\build-targets.py --package_name "OpenCV CPP" --package_version "0.0.1" --native_build_path "C:\opencv-cpp-build" --packages_path "C:\opencv-cpp-build" --sources_path "D:\Github\opencv"
import os, sys, argparse


def generate_property_group(list, args):
    files_list = ['<PropertyGroup>']
    files_list.append(f'<VisualStudioVersion Condition="\'$(VisualStudioVersion)\' == \'\'">10.0</VisualStudioVersion>')
    files_list.append(f'<LibraryType Condition="\'$(Configuration)\'==\'Debug\'">d</LibraryType>')
    files_list.append(f'<LibraryType Condition="\'$(Configuration)\'==\'Release\'"></LibraryType>')
    files_list.append('</PropertyGroup>')

    list += files_list

def generate_item_definition_group(list, args):
    files_list = ['<ItemDefinitionGroup>']
    files_list.append(f'<ClCompile>')
    files_list.append(f'<AdditionalIncludeDirectories>$(MSBuildThisFileDirectory)\\include\\;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>')
    files_list.append(f'</ClCompile>')
    files_list.append('</ItemDefinitionGroup>')

    list += files_list

def generate_item_definition_group_2(list, args):
    files_list = ['<ItemDefinitionGroup>']
    files_list.append(f'<Link>')
    files_list.append(f'<AdditionalLibraryDirectories>$(MSBuildThisFileDirectory)\\x$(Platform)\\vc$(VisualStudioVersion)\\lib\\;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>')
    files_list.append(f'<AdditionalDependencies>opencv_world*$(LibraryType).lib;%(AdditionalDependencies)</AdditionalDependencies>')
    files_list.append(f'</Link>')
    files_list.append('</ItemDefinitionGroup>')

    list += files_list

def generate_item_group(list, args):
    files_list = ['<ItemGroup>']
    files_list.append(f'<None Include="$(MSBuildThisFileDirectory)\\x64\\vc$(VisualStudioVersion)\\bin\\opencv_world*$(LibraryType).dll">')
    files_list.append(f'<Link>opencv_world*$(LibraryType).dll</Link>')
    files_list.append(f'<CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>')
    files_list.append(f'<Visible>false</Visible>')
    files_list.append(f'</None>')
    files_list.append('</ItemGroup>')

    list += files_list


def generate_targets(args):
    lines = ['<?xml version="1.0"?>']
    lines.append(f'<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003" ToolsVersion="15.0">')
    generate_property_group(lines, args)
    generate_item_definition_group(lines, args)
    generate_item_definition_group_2(lines, args)
    generate_item_group(lines, args)
    lines.append(f'</Project>')
    return lines


def main():

    args = {}
    print("started parser")

    # Generate targets
    lines = generate_targets(args)

    # Create the nuspec needed to generate the Nuget
    with open(os.path.join('C:\opencv-cpp-build', 'opencv-cpp.targets'), 'w') as f:
        for line in lines:
            f.write(line)
            f.write('\n')

    print("stopped parser")

if __name__ == "__main__":
    sys.exit(main())