#!/usr/bin/python3 -i
#
# Copyright (c) 2020-2025 The Khronos Group Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import sys
import os
import json
from vulkan_object import SpirvEnables
from base_generator import BaseGenerator
from generators.generator_utils import IsNonVulkanSprivCapability

#
# Generate SPIR-V validation for SPIR-V extensions and capabilities
class SpirvValidationHelperOutputGenerator(BaseGenerator):
    def __init__(self, grammar):
        BaseGenerator.__init__(self)

        # Sometimes the Vulkan-Headers XML will mention new SPIR-V capability or extensions
        # that require an update of the SPIRV-Headers which might not be ready to pull in.
        # Get the list of safe enum values to use from the SPIR-V grammar
        self.capabilityList = []
        self.provisionalList = []
        with open(grammar) as grammar_file:
            grammar_dict = json.load(grammar_file)
        for kind in grammar_dict['operand_kinds']:
            if kind['kind'] == 'Capability':
                for enum in kind['enumerants']:
                    if not IsNonVulkanSprivCapability(enum['enumerant']):
                        self.capabilityList.append(enum['enumerant'])
                    if 'provisional' in enum:
                        self.provisionalList.append(enum['enumerant'])
                break

        # Promoted features structure in state_tracker.cpp are put in the VkPhysicalDeviceVulkan*Features structs
        # but the XML can still list them. This list all promoted structs to ignore since they are aliased.
        # Tried to generate these, but no reliable way from vk.xml
        self.promotedFeatures = [
            # 1.1
            "VkPhysicalDevice16BitStorageFeatures",
            "VkPhysicalDeviceMultiviewFeatures",
            "VkPhysicalDeviceVariablePointersFeatures",
            "VkPhysicalDeviceProtectedMemoryFeatures",
            "VkPhysicalDeviceSamplerYcbcrConversionFeatures",
            "VkPhysicalDeviceShaderDrawParametersFeatures",
            # 1.2
            "VkPhysicalDevice8BitStorageFeatures",
            "VkPhysicalDeviceShaderFloat16Int8Features",
            "VkPhysicalDeviceDescriptorIndexingFeatures",
            "VkPhysicalDeviceScalarBlockLayoutFeatures",
            "VkPhysicalDeviceImagelessFramebufferFeatures",
            "VkPhysicalDeviceUniformBufferStandardLayoutFeatures",
            "VkPhysicalDeviceShaderSubgroupExtendedTypesFeatures",
            "VkPhysicalDeviceSeparateDepthStencilLayoutsFeatures",
            "VkPhysicalDeviceTimelineSemaphoreFeatures",
            "VkPhysicalDeviceBufferDeviceAddressFeatures",
            "VkPhysicalDeviceShaderAtomicInt64Features",
            "VkPhysicalDeviceVulkanMemoryModelFeatures",
            # 1.3
            "VkPhysicalDeviceShaderDemoteToHelperInvocationFeatures",
            "VkPhysicalDeviceShaderIntegerDotProductFeatures",
        ]

        # Properties are harder to handle genearted without generating a template for every property struct type
        # The simpler solution is create strings that will be printed out as static comparisons at compile time
        # The Map is used to map Vulkan property structs with the state tracker variable name
        self.propertyInfo = dict()
        self.propertyMap = {
            'VkPhysicalDeviceVulkan11Properties' : 'phys_dev_props_core11',
            'VkPhysicalDeviceVulkan12Properties' : 'phys_dev_props_core12',
            'VkPhysicalDeviceVulkan13Properties' : 'phys_dev_props_core13', # Not used, but left for grep searching
        }

    #
    # Creates the value of the struct declared in RequiredSpirvInfo
    def createMapValue(self, name: str, enable: SpirvEnables, isExtension: bool) -> str:
        out = []
        if enable.version is not None:
            # Version should be VK_VERSION_x_x as defined in header but need to get as VK_API_VERSION_x_x
            version = enable.version.replace('VK_VERSION', 'VK_API_VERSION')
            if name == 'SPV_KHR_non_semantic_info':
                version = 'VK_API_VERSION_1_0'
            out.append(f'{{{version}, nullptr, nullptr, ""}}')
        elif enable.feature is not None:
            out.append(f'{{0, &DeviceFeatures::{enable.feature}, nullptr, ""}}')
        elif enable.extension is not None:
            # All fields in DeviceExtensions should just be the extension name lowercase
            out.append(f'{{0, nullptr, &DeviceExtensions::{enable.extension.lower()}, ""}}')
        elif enable.property is not None:
            propertyStruct = enable.property
            # Need to make sure to return a boolean value to prevent compiler warning for implicit conversions
            propertyLogic = f'({propertyStruct}::{enable.member} & {enable.value}) != 0'
            # Property might have multiple items per capability/extension
            if name not in self.propertyInfo:
                self.propertyInfo[name] = []
            # Save info later to be printed out
            self.propertyInfo[name].append({
                "logic" : propertyLogic,
                "struct" : propertyStruct,
                "isExtension" : isExtension
            })
            # For properties, this string is just for human readableness
            out.append(f'{{0, nullptr, nullptr, "{propertyLogic}"}}')
        else:
            out.append('{0, nullptr, nullptr, ""}')
        return "".join(out)

    def generate(self):
        self.write(f'''// *** THIS FILE IS GENERATED - DO NOT EDIT ***
            // See {os.path.basename(__file__)} for modifications

            /***************************************************************************
            *
            * Copyright (c) 2020-2025 The Khronos Group Inc.
            *
            * Licensed under the Apache License, Version 2.0 (the "License");
            * you may not use this file except in compliance with the License.
            * You may obtain a copy of the License at
            *
            *     http://www.apache.org/licenses/LICENSE-2.0
            *
            * Unless required by applicable law or agreed to in writing, software
            * distributed under the License is distributed on an "AS IS" BASIS,
            * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
            * See the License for the specific language governing permissions and
            * limitations under the License.
            *
            * This file is related to anything that is found in the Vulkan XML related
            * to SPIR-V. Anything related to the SPIR-V grammar belongs in spirv_grammar_helper
            *
            ****************************************************************************/
            ''')
        self.write('// NOLINTBEGIN') # Wrap for clang-tidy to ignore

        if self.filename == 'spirv_validation_helper.h':
            self.generateHeader()
        elif self.filename == 'spirv_validation_helper.cpp':
            self.generateSource()
        else:
            self.write(f'\nFile name {self.filename} has no code to generate\n')

        self.write('// NOLINTEND') # Wrap for clang-tidy to ignore

    def generateHeader(self):
        out = []
        out.append('''
            #pragma once
            #include <vulkan/vulkan_core.h>

            // This is the one function that requires mapping SPIR-V enums to Vulkan enums
            VkFormat CompatibleSpirvImageFormat(uint32_t spirv_image_format);
            // Since we keep things in VkFormat for checking, we need a way to get the original SPIR-V
            // Format name for any error message
            const char* string_SpirvImageFormat(VkFormat format);
        ''')

        self.write("".join(out))

    def generateSource(self):
        out = []
        out.append('''
            #include <string>
            #include <string_view>
            #include <functional>
            #include <spirv/unified1/spirv.hpp>
            #include "vk_extension_helper.h"
            #include "state_tracker/shader_instruction.h"
            #include "state_tracker/shader_module.h"
            #include "stateless/sl_spirv.h"
            #include "generated/device_features.h"
            #include "error_message/error_location.h"
            ''')

        #
        # Creates the FeaturePointer struct to map features with those in the layers state tracker
        out.append('''
            struct FeaturePointer {
                // Callable object to test if this feature is enabled in the given aggregate feature struct
                const std::function<bool(const DeviceFeatures &)> IsEnabled;

                // Test if feature pointer is populated
                explicit operator bool() const { return static_cast<bool>(IsEnabled); }

                // Default and nullptr constructor to create an empty FeaturePointer
                FeaturePointer() : IsEnabled(nullptr) {}
                FeaturePointer(std::nullptr_t ptr) : IsEnabled(nullptr) {}
                FeaturePointer(bool DeviceFeatures::*ptr)
                    : IsEnabled([=](const DeviceFeatures &features) { return features.*ptr; }) {}
            };
            ''')

        out.append('''
            // Each instance of the struct will only have a singel field non-null
            struct RequiredSpirvInfo {
                uint32_t version;
                FeaturePointer feature;
                ExtEnabled DeviceExtensions::*extension;
                const char* property; // For human readability and make some capabilities unique
            };

            ''')

        #
        # Build the struct with all the requirements for the spirv capabilities
        out.append('const std::unordered_multimap<uint32_t, RequiredSpirvInfo>& GetSpirvCapabilites() {\n')
        out.append('// clang-format off\n')
        out.append('    static const std::unordered_multimap<uint32_t, RequiredSpirvInfo> spirv_capabilities = {')
        for spirv in [x for x in self.vk.spirv if x.capability]:
            for enable in [x for x in spirv.enable if x.struct is None or x.struct not in self.promotedFeatures]:
                if spirv.name not in self.capabilityList:
                    out.append('\n        // Not found in current SPIR-V Headers\n        // ')
                elif spirv.name in self.provisionalList:
                    out.append('\n#ifdef VK_ENABLE_BETA_EXTENSIONS\n        ')
                else:
                    out.append('\n        ')
                out.append(f'{{spv::Capability{spirv.name}, {self.createMapValue(spirv.name, enable, False)}}},')

                out.append('\n#endif') if spirv.name in self.provisionalList else None
        out.append('\n    };\n')
        out.append('// clang-format on\n')
        out.append('    return spirv_capabilities;\n')
        out.append('};\n')
        out.append('\n')

        #
        # Build the struct with all the requirements for the spirv extensions
        out.append('const std::unordered_multimap<std::string_view, RequiredSpirvInfo>& GetSpirvExtensions() {\n')
        out.append('// clang-format off\n')
        out.append('    static const std::unordered_multimap<std::string_view, RequiredSpirvInfo> spirv_extensions = {')
        for spirv in [x for x in self.vk.spirv if x.extension]:
            for enable in spirv.enable:
                if spirv.name == 'SPV_KHR_non_semantic_info' and enable.version is not None:
                    out.append('\n        // Debug Printf is in charge of adding SPV_KHR_non_semantic_info and emitting an error if it is not available.\n')
                    out.append('        // Given how things are wired, core check will not see the extension being added and would emit an error\n')
                    out.append('        // if SPV_KHR_non_semantic_info is then seen in some spirv code. To prevent that, assume the extension is always available.\n')
                    out.append('        // If Debug Printf is off, in practice even driver not supporting this extension will not crash.')
                out.append(f'\n        {{"{spirv.name}", {self.createMapValue(spirv.name, enable, True)}}},')
        out.append('\n    };\n')
        out.append('// clang-format on\n')
        out.append('    return spirv_extensions;\n')
        out.append('}\n')
        out.append('\n')

        #
        # Creates the Enum string helpers for better error messages. Same idea of vk_enum_string_helper.h but for SPIR-V
        out.append('static inline const char* string_SpvCapability(uint32_t input_value) {\n')
        out.append('    switch ((spv::Capability)input_value) {\n')
        for name in self.capabilityList:
            out.append('#ifdef VK_ENABLE_BETA_EXTENSIONS\n') if name in self.provisionalList else None
            out.append(f'         case spv::Capability{name}:\n')
            out.append(f'            return "{name}";\n')
            out.append('#endif\n') if name in self.provisionalList else None
        out.append('        default:\n')
        out.append('            return \"Unhandled OpCapability\";\n')
        out.append('    };\n')
        out.append('}\n')
        #
        # Creates SPIR-V image format helper
        out.append('''
            // Will return the Vulkan format for a given SPIR-V image format value
            // Note: will return VK_FORMAT_UNDEFINED if non valid input (or if ImageFormatUnknown)
            // This was in vk_format_utils but the SPIR-V Header dependency was an issue
            //   see https://github.com/KhronosGroup/Vulkan-ValidationLayers/pull/4647
            VkFormat CompatibleSpirvImageFormat(uint32_t spirv_image_format) {
                switch (spirv_image_format) {
            ''')
        for format in [x for x in self.vk.formats.values() if x.spirvImageFormat]:
            out.append(f'        case spv::ImageFormat{format.spirvImageFormat}:\n')
            out.append(f'            return {format.name};\n')
        out.append('        default:\n')
        out.append('            return VK_FORMAT_UNDEFINED;\n')
        out.append('    };\n')
        out.append('}\n')

        out.append('''
            const char* string_SpirvImageFormat(VkFormat format) {
                switch (format) {
            ''')
        for format in [x for x in self.vk.formats.values() if x.spirvImageFormat]:
            out.append(f'        case {format.name}:\n')
            out.append(f'            return \"{format.spirvImageFormat}\";\n')
        out.append('        default:\n')
        out.append('            return "Unknown SPIR-V Format";\n')
        out.append('    };\n')
        out.append('}\n')

        out.append('''
// clang-format off
static inline const char* SpvCapabilityRequirements(uint32_t capability) {
    static const vvl::unordered_map<uint32_t, std::string_view> table {
''')
        for spirv in [x for x in self.vk.spirv if x.capability and x.name in self.capabilityList]:
            requirment = ''
            for index, enable in enumerate([x for x in spirv.enable if x.struct is None or x.struct not in self.promotedFeatures]):
                requirment += ' OR ' if (index != 0) else ''
                if enable.version is not None:
                    requirment += enable.version
                elif enable.feature is not None:
                    requirment += f'{enable.struct}::{enable.feature}'
                elif enable.extension is not None:
                    requirment += enable.extension
                elif enable.property is not None:
                    requirment += f'({enable.property}::{enable.member} == {enable.value})'

            out.append('#ifdef VK_ENABLE_BETA_EXTENSIONS\n') if spirv.name in self.provisionalList else None
            out.append(f'    {{spv::Capability{spirv.name}, "{requirment}"}},\n')
            out.append('#endif  // VK_ENABLE_BETA_EXTENSIONS\n') if spirv.name in self.provisionalList else None
        out.append('''    };

    // VUs before catch unknown capabilities
    const auto entry = table.find(capability);
    return entry->second.data();
}
// clang-format on
''')

        out.append('''
// clang-format off
static inline std::string SpvExtensionRequirements(std::string_view extension) {
    static const vvl::unordered_map<std::string_view, vvl::Requirements> table {
''')
        for spirv in [x for x in self.vk.spirv if x.extension]:
            requirment = ''
            for index, enable in enumerate(spirv.enable):
                requirment += ', ' if (index != 0) else ''
                if enable.version is not None:
                    requirment += f'{{vvl::Version::_{enable.version}}}'
                elif enable.extension is not None:
                    requirment += f'{{vvl::Extension::_{enable.extension}}}'
                elif enable.feature is not None or enable.property is not None:
                    print("Need to add support for feature/properties in spirv extensions")
                    sys.exit(1)
            out.append(f'    {{"{spirv.name}", {{{requirment}}}}},\n')
        out.append('''    };

    // VUs before catch unknown extensions
    const auto entry = table.find(extension);
    return String(entry->second);
}
// clang-format on
''')

        # The chance of an SPIR-V extension having a property as a requirement is low
        # Instead of writting complex (and more confusing) code, just go back match what
        # we do for capabilities if one is ever added to the XML
        if len([infos for infos in self.propertyInfo.values() if infos[0]['isExtension']]) > 0:
            print("Error: XML has added a property requirement to a SPIR-V Extension")
            sys.exit(1)

        #
        # The main function to validate all the extensions and capabilities
        out.append('''
            bool stateless::SpirvValidator::ValidateShaderCapabilitiesAndExtensions(const spirv::Module& module_state, const spirv::Instruction &insn,  const Location& loc) const {
                bool skip = false;
                const bool pipeline = loc.function != vvl::Func::vkCreateShadersEXT;

                if (insn.Opcode() == spv::OpCapability) {
                    const uint32_t insn_capability = insn.Word(1);
                    // All capabilities are generated so if it is not in the list it is not supported by Vulkan
                    if (GetSpirvCapabilites().count(insn_capability) == 0) {
                        const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08739" : "VUID-VkShaderCreateInfoEXT-pCode-08739";
                        skip |= LogError(vuid, module_state.handle(), loc,
                            "SPIR-V has Capability (%s) declared, but this is not supported by Vulkan.", string_SpvCapability(insn_capability));
                        return skip; // no known capability to validate
                    }

                    // Each capability has one or more requirements to check
                    // Only one item has to be satisfied and an error only occurs
                    // when all are not satisfied
                    auto caps = GetSpirvCapabilites().equal_range(insn_capability);
                    bool has_support = false;
                    for (auto it = caps.first; (it != caps.second) && (has_support == false); ++it) {
                        if (it->second.version) {
                            if (api_version >= it->second.version) {
                                has_support = true;
                            }
                        } else if (it->second.feature) {
                            if (it->second.feature.IsEnabled(enabled_features)) {
                                has_support = true;
                            }
                        } else if (it->second.extension) {
                            // kEnabledByApiLevel is not valid as some extension are promoted with feature bits to be used.
                            // If the new Api Level gives support, it will be caught in the "it->second.version" check instead.
                            if (IsExtEnabledByCreateinfo(extensions.*(it->second.extension))) {
                                has_support = true;
                            }
                        } else if (it->second.property) {
                            // support is or'ed as only one has to be supported (if applicable)
                            switch (insn_capability) {''')

        for name, infos in sorted(self.propertyInfo.items()):
            # Only capabilities here (all items in array are the same)
            if infos[0]['isExtension']:
                continue

            # use triple-tick syntax to keep tab alignment for generated code
            out.append(f'''
                    case spv::Capability{name}:''')
            for info in infos:
                # Need to string replace property string to create valid C++ logic
                logic = info['logic'].replace('::', '.')
                logic = logic.replace(info['struct'], self.propertyMap[info['struct']])
                out.append(f'''
                        has_support |= ({logic});''')
            out.append('''
                        break;''')

        out.append('''
                        default:
                            break;
                    }
                }
            }
            ''')

        out.append('''
                if (!has_support && insn_capability == spv::CapabilityShaderViewportIndexLayerEXT) {
                    // special case (https://github.com/KhronosGroup/Vulkan-ValidationLayers/issues/9601)
                    if (enabled_features.shaderOutputLayer && enabled_features.shaderOutputViewportIndex) {
                        has_support = true;
                    }
                }

                if (has_support == false) {
                    const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08740" : "VUID-VkShaderCreateInfoEXT-pCode-08740";
                    skip |= LogError(vuid, module_state.handle(), loc,
                        "SPIR-V Capability %s was declared, but one of the following requirements is required (%s).", string_SpvCapability(insn_capability), SpvCapabilityRequirements(insn_capability));
                }

                // Portability checks
                if (IsExtEnabled(extensions.vk_khr_portability_subset)) {
                    if ((VK_FALSE == enabled_features.shaderSampleRateInterpolationFunctions) &&
                        (spv::CapabilityInterpolationFunction == insn_capability)) {
                        skip |= LogError("VUID-RuntimeSpirv-shaderSampleRateInterpolationFunctions-06325", module_state.handle(), loc,
                                            "SPIR-V (portability error) InterpolationFunction Capability are not supported "
                                            "by this platform");
                    }
                }
            } else if (insn.Opcode() == spv::OpExtension) {
                static const std::string spv_prefix = "SPV_";
                std::string extension_name = insn.GetAsString(1);

                if (0 == extension_name.compare(0, spv_prefix.size(), spv_prefix)) {
                    if (GetSpirvExtensions().count(extension_name) == 0) {
                        const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08741" : "VUID-VkShaderCreateInfoEXT-pCode-08741";
                        skip |= LogError(vuid, module_state.handle(), loc,
                            "SPIR-V Extension %s was declared, but that is not supported by Vulkan.", extension_name.c_str());
                        return skip; // no known extension to validate
                    }
                } else {
                    const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08741" : "VUID-VkShaderCreateInfoEXT-pCode-08741";
                    skip |= LogError(vuid, module_state.handle(), loc,
                        "SPIR-V Extension %s was declared, but this is not a SPIR-V extension. Please use a SPIR-V"
                        " extension (https://github.com/KhronosGroup/SPIRV-Registry) for OpExtension instructions. Non-SPIR-V extensions can be"
                        " recorded in SPIR-V using the OpSourceExtension instruction.", extension_name.c_str());
                    return skip; // no known extension to validate
                }

                // Each SPIR-V Extension has one or more requirements to check
                // Only one item has to be satisfied and an error only occurs
                // when all are not satisfied
                auto ext = GetSpirvExtensions().equal_range(extension_name);
                bool has_support = false;
                for (auto it = ext.first; (it != ext.second) && (has_support == false); ++it) {
                    if (it->second.version) {
                        if (api_version >= it->second.version) {
                            has_support = true;
                        }
                    } else if (it->second.feature) {
                        if (it->second.feature.IsEnabled(enabled_features)) {
                            has_support = true;
                        }
                    } else if (it->second.extension) {
                        if (IsExtEnabled(extensions.*(it->second.extension))) {
                            has_support = true;
                        }
                    }
                }

                if (has_support == false) {
                    const char *vuid = pipeline ? "VUID-VkShaderModuleCreateInfo-pCode-08742" : "VUID-VkShaderCreateInfoEXT-pCode-08742";
                    skip |= LogError(vuid, module_state.handle(), loc,
                        "SPIR-V Extension %s was declared, but one of the following requirements is required (%s).", extension_name.c_str(), SpvExtensionRequirements(extension_name).c_str());
                }
            } //spv::OpExtension
            return skip;
            }
            ''')
        self.write("".join(out))
