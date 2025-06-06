/* Copyright (c) 2025 The Khronos Group Inc.
 * Copyright (c) 2025 Valve Corporation
 * Copyright (c) 2025 LunarG, Inc.
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
 */

#include "gpuav/core/gpuav.h"
#include "generated/dispatch_functions.h"

namespace gpuav {

static std::vector<VkExtensionProperties> GetAvailableExtensions(VkPhysicalDevice physical_device) {
    VkResult err;
    uint32_t extension_count = 512;
    std::vector<VkExtensionProperties> extensions(extension_count);
    for (;;) {
        err = DispatchEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_count, extensions.data());
        if (err == VK_SUCCESS) {
            extensions.resize(extension_count);
            return extensions;
        } else if (err == VK_INCOMPLETE) {
            extension_count *= 2;  // wasn't enough space, increase it
            extensions.resize(extension_count);
        } else {
            return {};
        }
    }
}

static bool IsExtensionAvailable(const char *extension_name, const std::vector<VkExtensionProperties> &available_extensions) {
    for (const VkExtensionProperties &ext : available_extensions) {
        if (strncmp(extension_name, ext.extensionName, VK_MAX_EXTENSION_NAME_SIZE) == 0) {
            return true;
        }
    }

    return false;
}

// We end up printing lots of warning for GPU-AV that really have no use for any object.
// Instead of spamming with a VkInstance, just explicitly don't print anything
static const VulkanTypedHandle kNoObjects;

// In PreCallRecord we try to turn on as many features as possible on behalf of the app (and warn the user if we are doing it).
// Later after device creation, we can decide if the required Vulkan feature for each GPU-AV setting is found and report errors
void Instance::AddFeatures(VkPhysicalDevice physical_device, vku::safe_VkDeviceCreateInfo *modified_create_info,
                           const Location &loc) {
    // Query things here to make sure we don't attempt to add a feature this is just not supported
    VkPhysicalDeviceRobustness2FeaturesEXT supported_robustness2_feature = vku::InitStructHelper();
    VkPhysicalDevice8BitStorageFeatures supported_8bit_feature = vku::InitStructHelper(&supported_robustness2_feature);
    VkPhysicalDeviceBufferDeviceAddressFeatures supported_bda_feature = vku::InitStructHelper(&supported_8bit_feature);
    VkPhysicalDeviceVulkanMemoryModelFeatures supported_memory_model_feature = vku::InitStructHelper(&supported_bda_feature);
    VkPhysicalDeviceTimelineSemaphoreFeatures supported_timeline_feature = vku::InitStructHelper(&supported_memory_model_feature);
    VkPhysicalDeviceFeatures2 features_2 = vku::InitStructHelper(&supported_timeline_feature);
    DispatchGetPhysicalDeviceFeatures2(physical_device, &features_2);

    // First core features
    {
        const VkPhysicalDeviceFeatures &supported_features = features_2.features;

        VkPhysicalDeviceFeatures *modified_features =
            const_cast<VkPhysicalDeviceFeatures *>(modified_create_info->pEnabledFeatures);
        if (!modified_features) {
            if (auto *modified_features_2 = const_cast<VkPhysicalDeviceFeatures2 *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceFeatures2>(modified_create_info->pNext))) {
                modified_features = &modified_features_2->features;
            } else {
                // The user has no VkPhysicalDeviceFeatures, so we are adding it for them
                modified_features = new VkPhysicalDeviceFeatures;
                memset(modified_features, 0, sizeof(VkPhysicalDeviceFeatures));
                modified_create_info->pEnabledFeatures = modified_features;
            }
        }

        if (modified_features) {
            if (supported_features.fragmentStoresAndAtomics && !modified_features->fragmentStoresAndAtomics) {
                InternalWarning(kNoObjects, loc, "Forcing fragmentStoresAndAtomics to VK_TRUE");
                modified_features->fragmentStoresAndAtomics = VK_TRUE;
            }
            if (supported_features.vertexPipelineStoresAndAtomics && !modified_features->vertexPipelineStoresAndAtomics) {
                InternalWarning(kNoObjects, loc, "Forcing vertexPipelineStoresAndAtomics to VK_TRUE");
                modified_features->vertexPipelineStoresAndAtomics = VK_TRUE;
            }
            if (supported_features.shaderInt64 && !modified_features->shaderInt64) {
                InternalWarning(kNoObjects, loc, "Forcing shaderInt64 to VK_TRUE");
                modified_features->shaderInt64 = VK_TRUE;
            }
            if (gpuav_settings.force_on_robustness && supported_features.robustBufferAccess &&
                !modified_features->robustBufferAccess) {
                InternalWarning(kNoObjects, loc, "Forcing robustBufferAccess to VK_TRUE");
                modified_features->robustBufferAccess = VK_TRUE;
            }
        }
    }

    // Build extension list once
    std::vector<VkExtensionProperties> available_extensions = GetAvailableExtensions(physical_device);

    if (supported_timeline_feature.timelineSemaphore) {
        auto add_timeline_semaphore = [this, &loc, modified_create_info]() {
            if (auto *ts_features = const_cast<VkPhysicalDeviceTimelineSemaphoreFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceTimelineSemaphoreFeatures>(modified_create_info))) {
                if (ts_features->timelineSemaphore == VK_FALSE) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDeviceTimelineSemaphoreFeatures::timelineSemaphore to VK_TRUE");
                    ts_features->timelineSemaphore = VK_TRUE;
                }
            } else {
                InternalWarning(
                    kNoObjects, loc,
                    "Adding a VkPhysicalDeviceTimelineSemaphoreFeatures to pNext with timelineSemaphore set to VK_TRUE");
                VkPhysicalDeviceTimelineSemaphoreFeatures new_ts_features = vku::InitStructHelper();
                new_ts_features.timelineSemaphore = VK_TRUE;
                vku::AddToPnext(*modified_create_info, new_ts_features);
            }
        };

        if (api_version > VK_API_VERSION_1_1) {
            if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
                if (features12->timelineSemaphore == VK_FALSE) {
                    InternalWarning(kNoObjects, loc, "Forcing VkPhysicalDeviceVulkan12Features::timelineSemaphore to VK_TRUE");
                    features12->timelineSemaphore = VK_TRUE;
                }
            } else {
                add_timeline_semaphore();
            }
        } else if (IsExtensionAvailable(VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME, available_extensions)) {
            // Only adds if not found already
            vku::AddExtension(*modified_create_info, VK_KHR_TIMELINE_SEMAPHORE_EXTENSION_NAME);
            add_timeline_semaphore();
            timeline_khr_ = true;
        }
    }

    if (supported_memory_model_feature.vulkanMemoryModel) {
        auto add_memory_model = [this, &loc, modified_create_info]() {
            if (auto *mm_features = const_cast<VkPhysicalDeviceVulkanMemoryModelFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkanMemoryModelFeatures>(modified_create_info))) {
                if (mm_features->vulkanMemoryModel == VK_FALSE) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModel to VK_TRUE");
                    mm_features->vulkanMemoryModel = VK_TRUE;
                }
                if (mm_features->vulkanMemoryModelDeviceScope == VK_FALSE) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDeviceVulkanMemoryModelFeatures::vulkanMemoryModelDeviceScope to VK_TRUE");
                    mm_features->vulkanMemoryModelDeviceScope = VK_TRUE;
                }
            } else {
                InternalWarning(kNoObjects, loc,
                                "Adding a VkPhysicalDeviceVulkanMemoryModelFeatures to pNext with vulkanMemoryModel and "
                                "vulkanMemoryModelDeviceScope set to VK_TRUE");
                VkPhysicalDeviceVulkanMemoryModelFeatures new_mm_features = vku::InitStructHelper();
                new_mm_features.vulkanMemoryModel = VK_TRUE;
                new_mm_features.vulkanMemoryModelDeviceScope = VK_TRUE;
                vku::AddToPnext(*modified_create_info, new_mm_features);
            }
        };

        if (api_version > VK_API_VERSION_1_1) {
            if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
                if (features12->vulkanMemoryModel == VK_FALSE) {
                    InternalWarning(kNoObjects, loc, "Forcing VkPhysicalDeviceVulkan12Features::vulkanMemoryModel to VK_TRUE");
                    features12->vulkanMemoryModel = VK_TRUE;
                }
                if (features12->vulkanMemoryModelDeviceScope == VK_FALSE) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDeviceVulkan12Features::vulkanMemoryModelDeviceScope to VK_TRUE");
                    features12->vulkanMemoryModelDeviceScope = VK_TRUE;
                }
            } else {
                add_memory_model();
            }
        } else if (IsExtensionAvailable(VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME, available_extensions)) {
            // Only adds if not found already
            vku::AddExtension(*modified_create_info, VK_KHR_VULKAN_MEMORY_MODEL_EXTENSION_NAME);
            add_memory_model();
        }
    }

    if (supported_bda_feature.bufferDeviceAddress) {
        auto add_bda = [this, &loc, modified_create_info]() {
            // Add buffer device address feature
            if (auto *bda_features = const_cast<VkPhysicalDeviceBufferDeviceAddressFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceBufferDeviceAddressFeatures>(modified_create_info))) {
                if (!bda_features->bufferDeviceAddress) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDeviceBufferDeviceAddressFeatures::bufferDeviceAddress to VK_TRUE");
                    bda_features->bufferDeviceAddress = VK_TRUE;
                }
            } else {
                InternalWarning(
                    kNoObjects, loc,
                    "Adding a VkPhysicalDeviceBufferDeviceAddressFeatures to pNext with bufferDeviceAddress set to VK_TRUE");
                VkPhysicalDeviceBufferDeviceAddressFeatures new_bda_features = vku::InitStructHelper();
                new_bda_features.bufferDeviceAddress = VK_TRUE;
                vku::AddToPnext(*modified_create_info, new_bda_features);
            }
        };

        if (api_version >= VK_API_VERSION_1_2) {
            if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
                if (!features12->bufferDeviceAddress) {
                    InternalWarning(kNoObjects, loc, "Forcing VkPhysicalDeviceVulkan12Features::bufferDeviceAddress to VK_TRUE");
                    features12->bufferDeviceAddress = VK_TRUE;
                }
            } else {
                add_bda();
            }
        } else if (IsExtensionAvailable(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME, available_extensions)) {
            // Only adds if not found already
            vku::AddExtension(*modified_create_info, VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME);
            add_bda();
        }
    }

    if (supported_8bit_feature.storageBuffer8BitAccess) {
        auto add_8bit_access = [this, &loc, modified_create_info]() {
            // Add storageBuffer8BitAccess feature
            if (auto *eight_bit_access_feature = const_cast<VkPhysicalDevice8BitStorageFeatures *>(
                    vku::FindStructInPNextChain<VkPhysicalDevice8BitStorageFeatures>(modified_create_info))) {
                if (!eight_bit_access_feature->storageBuffer8BitAccess) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDevice8BitStorageFeatures::storageBuffer8BitAccess to VK_TRUE");
                    eight_bit_access_feature->storageBuffer8BitAccess = VK_TRUE;
                }
            } else {
                InternalWarning(kNoObjects, loc,
                                "Adding a VkPhysicalDevice8BitStorageFeatures to pNext with storageBuffer8BitAccess "
                                "set to VK_TRUE");
                VkPhysicalDevice8BitStorageFeatures new_8bit_features = vku::InitStructHelper();
                new_8bit_features.storageBuffer8BitAccess = VK_TRUE;
                vku::AddToPnext(*modified_create_info, new_8bit_features);
            }
        };

        if (api_version >= VK_API_VERSION_1_2) {
            if (auto *features12 = const_cast<VkPhysicalDeviceVulkan12Features *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceVulkan12Features>(modified_create_info->pNext))) {
                if (!features12->storageBuffer8BitAccess) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDeviceVulkan12Features::storageBuffer8BitAccess to VK_TRUE");
                    features12->storageBuffer8BitAccess = VK_TRUE;
                }
            } else {
                add_8bit_access();
            }
        } else if (IsExtensionAvailable(VK_KHR_8BIT_STORAGE_EXTENSION_NAME, available_extensions)) {
            // Only adds if not found already
            vku::AddExtension(*modified_create_info, VK_KHR_8BIT_STORAGE_EXTENSION_NAME);
            add_8bit_access();
        }
    }

    if (gpuav_settings.debug_printf_enabled) {
        if (!IsExtensionAvailable(VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME, available_extensions)) {
            InternalWarning(kNoObjects, loc,
                            "VK_KHR_shader_non_semantic_info is not available on selected device, Debug Printf may produce SPIR-V "
                            "that could fail to compile the shader.");
        } else {
            vku::AddExtension(*modified_create_info, VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME);
        }
    }

    if (gpuav_settings.force_on_robustness &&
        (supported_robustness2_feature.robustBufferAccess2 || supported_robustness2_feature.robustImageAccess2)) {
        if (IsExtensionAvailable(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME, available_extensions)) {
            // Only adds if not found already
            vku::AddExtension(*modified_create_info, VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

            if (auto *robust_buffer_2_feature = const_cast<VkPhysicalDeviceRobustness2FeaturesEXT *>(
                    vku::FindStructInPNextChain<VkPhysicalDeviceRobustness2FeaturesEXT>(modified_create_info))) {
                if (!robust_buffer_2_feature->robustBufferAccess2 && supported_robustness2_feature.robustBufferAccess2) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDeviceRobustness2FeaturesEXT::robustBufferAccess2 to VK_TRUE");
                    robust_buffer_2_feature->robustBufferAccess2 = VK_TRUE;
                }
                if (!robust_buffer_2_feature->robustImageAccess2 && supported_robustness2_feature.robustImageAccess2) {
                    InternalWarning(kNoObjects, loc,
                                    "Forcing VkPhysicalDeviceRobustness2FeaturesEXT::robustImageAccess2 to VK_TRUE");
                    robust_buffer_2_feature->robustImageAccess2 = VK_TRUE;
                }
            } else {
                VkPhysicalDeviceRobustness2FeaturesEXT new_robust_buffer_2_feature = vku::InitStructHelper();
                if (supported_robustness2_feature.robustBufferAccess2) {
                    InternalWarning(
                        kNoObjects, loc,
                        "Adding a VkPhysicalDeviceRobustness2FeaturesEXT to pNext with robustBufferAccess2 set to VK_TRUE");
                    new_robust_buffer_2_feature.robustBufferAccess2 = VK_TRUE;
                }
                if (supported_robustness2_feature.robustImageAccess2) {
                    InternalWarning(
                        kNoObjects, loc,
                        "Adding a VkPhysicalDeviceRobustness2FeaturesEXT to pNext with robustImageAccess2 set to VK_TRUE");
                    new_robust_buffer_2_feature.robustImageAccess2 = VK_TRUE;
                }
                vku::AddToPnext(*modified_create_info, new_robust_buffer_2_feature);
            }
        }
    }
}

}  // namespace gpuav
