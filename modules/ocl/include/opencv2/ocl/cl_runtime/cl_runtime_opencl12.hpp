//
// AUTOGENERATED, DO NOT EDIT
//
#ifndef __OPENCV_OCL_CL_RUNTIME_OPENCL_HPP__
#define __OPENCV_OCL_CL_RUNTIME_OPENCL_HPP__

#ifdef HAVE_OPENCL

#if defined __APPLE__ && !defined(IOS)
#include <OpenCL/cl.h>
#else

// generated by parser_cl.py
#define clGetPlatformIDs clGetPlatformIDs_
#define clGetPlatformInfo clGetPlatformInfo_
#define clGetDeviceIDs clGetDeviceIDs_
#define clGetDeviceInfo clGetDeviceInfo_
#define clCreateSubDevices clCreateSubDevices_
#define clRetainDevice clRetainDevice_
#define clReleaseDevice clReleaseDevice_
#define clCreateContext clCreateContext_
#define clCreateContextFromType clCreateContextFromType_
#define clRetainContext clRetainContext_
#define clReleaseContext clReleaseContext_
#define clGetContextInfo clGetContextInfo_
#define clCreateCommandQueue clCreateCommandQueue_
#define clRetainCommandQueue clRetainCommandQueue_
#define clReleaseCommandQueue clReleaseCommandQueue_
#define clGetCommandQueueInfo clGetCommandQueueInfo_
#define clCreateBuffer clCreateBuffer_
#define clCreateSubBuffer clCreateSubBuffer_
#define clCreateImage clCreateImage_
#define clRetainMemObject clRetainMemObject_
#define clReleaseMemObject clReleaseMemObject_
#define clGetSupportedImageFormats clGetSupportedImageFormats_
#define clGetMemObjectInfo clGetMemObjectInfo_
#define clGetImageInfo clGetImageInfo_
#define clSetMemObjectDestructorCallback clSetMemObjectDestructorCallback_
#define clCreateSampler clCreateSampler_
#define clRetainSampler clRetainSampler_
#define clReleaseSampler clReleaseSampler_
#define clGetSamplerInfo clGetSamplerInfo_
#define clCreateProgramWithSource clCreateProgramWithSource_
#define clCreateProgramWithBinary clCreateProgramWithBinary_
#define clCreateProgramWithBuiltInKernels clCreateProgramWithBuiltInKernels_
#define clRetainProgram clRetainProgram_
#define clReleaseProgram clReleaseProgram_
#define clBuildProgram clBuildProgram_
#define clCompileProgram clCompileProgram_
#define clLinkProgram clLinkProgram_
#define clUnloadPlatformCompiler clUnloadPlatformCompiler_
#define clGetProgramInfo clGetProgramInfo_
#define clGetProgramBuildInfo clGetProgramBuildInfo_
#define clCreateKernel clCreateKernel_
#define clCreateKernelsInProgram clCreateKernelsInProgram_
#define clRetainKernel clRetainKernel_
#define clReleaseKernel clReleaseKernel_
#define clSetKernelArg clSetKernelArg_
#define clGetKernelInfo clGetKernelInfo_
#define clGetKernelArgInfo clGetKernelArgInfo_
#define clGetKernelWorkGroupInfo clGetKernelWorkGroupInfo_
#define clWaitForEvents clWaitForEvents_
#define clGetEventInfo clGetEventInfo_
#define clCreateUserEvent clCreateUserEvent_
#define clRetainEvent clRetainEvent_
#define clReleaseEvent clReleaseEvent_
#define clSetUserEventStatus clSetUserEventStatus_
#define clSetEventCallback clSetEventCallback_
#define clGetEventProfilingInfo clGetEventProfilingInfo_
#define clFlush clFlush_
#define clFinish clFinish_
#define clEnqueueReadBuffer clEnqueueReadBuffer_
#define clEnqueueReadBufferRect clEnqueueReadBufferRect_
#define clEnqueueWriteBuffer clEnqueueWriteBuffer_
#define clEnqueueWriteBufferRect clEnqueueWriteBufferRect_
#define clEnqueueFillBuffer clEnqueueFillBuffer_
#define clEnqueueCopyBuffer clEnqueueCopyBuffer_
#define clEnqueueCopyBufferRect clEnqueueCopyBufferRect_
#define clEnqueueReadImage clEnqueueReadImage_
#define clEnqueueWriteImage clEnqueueWriteImage_
#define clEnqueueFillImage clEnqueueFillImage_
#define clEnqueueCopyImage clEnqueueCopyImage_
#define clEnqueueCopyImageToBuffer clEnqueueCopyImageToBuffer_
#define clEnqueueCopyBufferToImage clEnqueueCopyBufferToImage_
#define clEnqueueMapBuffer clEnqueueMapBuffer_
#define clEnqueueMapImage clEnqueueMapImage_
#define clEnqueueUnmapMemObject clEnqueueUnmapMemObject_
#define clEnqueueMigrateMemObjects clEnqueueMigrateMemObjects_
#define clEnqueueNDRangeKernel clEnqueueNDRangeKernel_
#define clEnqueueTask clEnqueueTask_
#define clEnqueueNativeKernel clEnqueueNativeKernel_
#define clEnqueueMarkerWithWaitList clEnqueueMarkerWithWaitList_
#define clEnqueueBarrierWithWaitList clEnqueueBarrierWithWaitList_
#define clGetExtensionFunctionAddressForPlatform clGetExtensionFunctionAddressForPlatform_
#define clCreateImage2D clCreateImage2D_
#define clCreateImage3D clCreateImage3D_
#define clEnqueueMarker clEnqueueMarker_
#define clEnqueueWaitForEvents clEnqueueWaitForEvents_
#define clEnqueueBarrier clEnqueueBarrier_
#define clUnloadCompiler clUnloadCompiler_
#define clGetExtensionFunctionAddress clGetExtensionFunctionAddress_

#if defined __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

// generated by parser_cl.py
#undef clGetPlatformIDs
#define clGetPlatformIDs clGetPlatformIDs_pfn
#undef clGetPlatformInfo
#define clGetPlatformInfo clGetPlatformInfo_pfn
#undef clGetDeviceIDs
#define clGetDeviceIDs clGetDeviceIDs_pfn
#undef clGetDeviceInfo
#define clGetDeviceInfo clGetDeviceInfo_pfn
#undef clCreateSubDevices
#define clCreateSubDevices clCreateSubDevices_pfn
#undef clRetainDevice
#define clRetainDevice clRetainDevice_pfn
#undef clReleaseDevice
#define clReleaseDevice clReleaseDevice_pfn
#undef clCreateContext
#define clCreateContext clCreateContext_pfn
#undef clCreateContextFromType
#define clCreateContextFromType clCreateContextFromType_pfn
#undef clRetainContext
#define clRetainContext clRetainContext_pfn
#undef clReleaseContext
#define clReleaseContext clReleaseContext_pfn
#undef clGetContextInfo
#define clGetContextInfo clGetContextInfo_pfn
#undef clCreateCommandQueue
#define clCreateCommandQueue clCreateCommandQueue_pfn
#undef clRetainCommandQueue
#define clRetainCommandQueue clRetainCommandQueue_pfn
#undef clReleaseCommandQueue
#define clReleaseCommandQueue clReleaseCommandQueue_pfn
#undef clGetCommandQueueInfo
#define clGetCommandQueueInfo clGetCommandQueueInfo_pfn
#undef clCreateBuffer
#define clCreateBuffer clCreateBuffer_pfn
#undef clCreateSubBuffer
#define clCreateSubBuffer clCreateSubBuffer_pfn
#undef clCreateImage
#define clCreateImage clCreateImage_pfn
#undef clRetainMemObject
#define clRetainMemObject clRetainMemObject_pfn
#undef clReleaseMemObject
#define clReleaseMemObject clReleaseMemObject_pfn
#undef clGetSupportedImageFormats
#define clGetSupportedImageFormats clGetSupportedImageFormats_pfn
#undef clGetMemObjectInfo
#define clGetMemObjectInfo clGetMemObjectInfo_pfn
#undef clGetImageInfo
#define clGetImageInfo clGetImageInfo_pfn
#undef clSetMemObjectDestructorCallback
#define clSetMemObjectDestructorCallback clSetMemObjectDestructorCallback_pfn
#undef clCreateSampler
#define clCreateSampler clCreateSampler_pfn
#undef clRetainSampler
#define clRetainSampler clRetainSampler_pfn
#undef clReleaseSampler
#define clReleaseSampler clReleaseSampler_pfn
#undef clGetSamplerInfo
#define clGetSamplerInfo clGetSamplerInfo_pfn
#undef clCreateProgramWithSource
#define clCreateProgramWithSource clCreateProgramWithSource_pfn
#undef clCreateProgramWithBinary
#define clCreateProgramWithBinary clCreateProgramWithBinary_pfn
#undef clCreateProgramWithBuiltInKernels
#define clCreateProgramWithBuiltInKernels clCreateProgramWithBuiltInKernels_pfn
#undef clRetainProgram
#define clRetainProgram clRetainProgram_pfn
#undef clReleaseProgram
#define clReleaseProgram clReleaseProgram_pfn
#undef clBuildProgram
#define clBuildProgram clBuildProgram_pfn
#undef clCompileProgram
#define clCompileProgram clCompileProgram_pfn
#undef clLinkProgram
#define clLinkProgram clLinkProgram_pfn
#undef clUnloadPlatformCompiler
#define clUnloadPlatformCompiler clUnloadPlatformCompiler_pfn
#undef clGetProgramInfo
#define clGetProgramInfo clGetProgramInfo_pfn
#undef clGetProgramBuildInfo
#define clGetProgramBuildInfo clGetProgramBuildInfo_pfn
#undef clCreateKernel
#define clCreateKernel clCreateKernel_pfn
#undef clCreateKernelsInProgram
#define clCreateKernelsInProgram clCreateKernelsInProgram_pfn
#undef clRetainKernel
#define clRetainKernel clRetainKernel_pfn
#undef clReleaseKernel
#define clReleaseKernel clReleaseKernel_pfn
#undef clSetKernelArg
#define clSetKernelArg clSetKernelArg_pfn
#undef clGetKernelInfo
#define clGetKernelInfo clGetKernelInfo_pfn
#undef clGetKernelArgInfo
#define clGetKernelArgInfo clGetKernelArgInfo_pfn
#undef clGetKernelWorkGroupInfo
#define clGetKernelWorkGroupInfo clGetKernelWorkGroupInfo_pfn
#undef clWaitForEvents
#define clWaitForEvents clWaitForEvents_pfn
#undef clGetEventInfo
#define clGetEventInfo clGetEventInfo_pfn
#undef clCreateUserEvent
#define clCreateUserEvent clCreateUserEvent_pfn
#undef clRetainEvent
#define clRetainEvent clRetainEvent_pfn
#undef clReleaseEvent
#define clReleaseEvent clReleaseEvent_pfn
#undef clSetUserEventStatus
#define clSetUserEventStatus clSetUserEventStatus_pfn
#undef clSetEventCallback
#define clSetEventCallback clSetEventCallback_pfn
#undef clGetEventProfilingInfo
#define clGetEventProfilingInfo clGetEventProfilingInfo_pfn
#undef clFlush
#define clFlush clFlush_pfn
#undef clFinish
#define clFinish clFinish_pfn
#undef clEnqueueReadBuffer
#define clEnqueueReadBuffer clEnqueueReadBuffer_pfn
#undef clEnqueueReadBufferRect
#define clEnqueueReadBufferRect clEnqueueReadBufferRect_pfn
#undef clEnqueueWriteBuffer
#define clEnqueueWriteBuffer clEnqueueWriteBuffer_pfn
#undef clEnqueueWriteBufferRect
#define clEnqueueWriteBufferRect clEnqueueWriteBufferRect_pfn
#undef clEnqueueFillBuffer
#define clEnqueueFillBuffer clEnqueueFillBuffer_pfn
#undef clEnqueueCopyBuffer
#define clEnqueueCopyBuffer clEnqueueCopyBuffer_pfn
#undef clEnqueueCopyBufferRect
#define clEnqueueCopyBufferRect clEnqueueCopyBufferRect_pfn
#undef clEnqueueReadImage
#define clEnqueueReadImage clEnqueueReadImage_pfn
#undef clEnqueueWriteImage
#define clEnqueueWriteImage clEnqueueWriteImage_pfn
#undef clEnqueueFillImage
#define clEnqueueFillImage clEnqueueFillImage_pfn
#undef clEnqueueCopyImage
#define clEnqueueCopyImage clEnqueueCopyImage_pfn
#undef clEnqueueCopyImageToBuffer
#define clEnqueueCopyImageToBuffer clEnqueueCopyImageToBuffer_pfn
#undef clEnqueueCopyBufferToImage
#define clEnqueueCopyBufferToImage clEnqueueCopyBufferToImage_pfn
#undef clEnqueueMapBuffer
#define clEnqueueMapBuffer clEnqueueMapBuffer_pfn
#undef clEnqueueMapImage
#define clEnqueueMapImage clEnqueueMapImage_pfn
#undef clEnqueueUnmapMemObject
#define clEnqueueUnmapMemObject clEnqueueUnmapMemObject_pfn
#undef clEnqueueMigrateMemObjects
#define clEnqueueMigrateMemObjects clEnqueueMigrateMemObjects_pfn
#undef clEnqueueNDRangeKernel
#define clEnqueueNDRangeKernel clEnqueueNDRangeKernel_pfn
#undef clEnqueueTask
#define clEnqueueTask clEnqueueTask_pfn
#undef clEnqueueNativeKernel
#define clEnqueueNativeKernel clEnqueueNativeKernel_pfn
#undef clEnqueueMarkerWithWaitList
#define clEnqueueMarkerWithWaitList clEnqueueMarkerWithWaitList_pfn
#undef clEnqueueBarrierWithWaitList
#define clEnqueueBarrierWithWaitList clEnqueueBarrierWithWaitList_pfn
#undef clGetExtensionFunctionAddressForPlatform
#define clGetExtensionFunctionAddressForPlatform clGetExtensionFunctionAddressForPlatform_pfn
#undef clCreateImage2D
#define clCreateImage2D clCreateImage2D_pfn
#undef clCreateImage3D
#define clCreateImage3D clCreateImage3D_pfn
#undef clEnqueueMarker
#define clEnqueueMarker clEnqueueMarker_pfn
#undef clEnqueueWaitForEvents
#define clEnqueueWaitForEvents clEnqueueWaitForEvents_pfn
#undef clEnqueueBarrier
#define clEnqueueBarrier clEnqueueBarrier_pfn
#undef clUnloadCompiler
#define clUnloadCompiler clUnloadCompiler_pfn
#undef clGetExtensionFunctionAddress
#define clGetExtensionFunctionAddress clGetExtensionFunctionAddress_pfn

#ifndef CL_RUNTIME_EXPORT
#if (defined(BUILD_SHARED_LIBS) || defined(OPENCV_OCL_SHARED)) && (defined WIN32 || defined _WIN32 || defined WINCE)
#define CL_RUNTIME_EXPORT __declspec(dllimport)
#else
#define CL_RUNTIME_EXPORT
#endif
#endif

// generated by parser_cl.py
extern CL_RUNTIME_EXPORT cl_int (*clGetPlatformIDs)(cl_uint, cl_platform_id*, cl_uint*);
extern CL_RUNTIME_EXPORT cl_int (*clGetPlatformInfo)(cl_platform_id, cl_platform_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clGetDeviceIDs)(cl_platform_id, cl_device_type, cl_uint, cl_device_id*, cl_uint*);
extern CL_RUNTIME_EXPORT cl_int (*clGetDeviceInfo)(cl_device_id, cl_device_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clCreateSubDevices)(cl_device_id, const cl_device_partition_property*, cl_uint, cl_device_id*, cl_uint*);
extern CL_RUNTIME_EXPORT cl_int (*clRetainDevice)(cl_device_id);
extern CL_RUNTIME_EXPORT cl_int (*clReleaseDevice)(cl_device_id);
extern CL_RUNTIME_EXPORT cl_context (*clCreateContext)(const cl_context_properties*, cl_uint, const cl_device_id*, void (CL_CALLBACK*) (const char*, const void*, size_t, void*), void*, cl_int*);
extern CL_RUNTIME_EXPORT cl_context (*clCreateContextFromType)(const cl_context_properties*, cl_device_type, void (CL_CALLBACK*) (const char*, const void*, size_t, void*), void*, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clRetainContext)(cl_context);
extern CL_RUNTIME_EXPORT cl_int (*clReleaseContext)(cl_context);
extern CL_RUNTIME_EXPORT cl_int (*clGetContextInfo)(cl_context, cl_context_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_command_queue (*clCreateCommandQueue)(cl_context, cl_device_id, cl_command_queue_properties, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clRetainCommandQueue)(cl_command_queue);
extern CL_RUNTIME_EXPORT cl_int (*clReleaseCommandQueue)(cl_command_queue);
extern CL_RUNTIME_EXPORT cl_int (*clGetCommandQueueInfo)(cl_command_queue, cl_command_queue_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_mem (*clCreateBuffer)(cl_context, cl_mem_flags, size_t, void*, cl_int*);
extern CL_RUNTIME_EXPORT cl_mem (*clCreateSubBuffer)(cl_mem, cl_mem_flags, cl_buffer_create_type, const void*, cl_int*);
extern CL_RUNTIME_EXPORT cl_mem (*clCreateImage)(cl_context, cl_mem_flags, const cl_image_format*, const cl_image_desc*, void*, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clRetainMemObject)(cl_mem);
extern CL_RUNTIME_EXPORT cl_int (*clReleaseMemObject)(cl_mem);
extern CL_RUNTIME_EXPORT cl_int (*clGetSupportedImageFormats)(cl_context, cl_mem_flags, cl_mem_object_type, cl_uint, cl_image_format*, cl_uint*);
extern CL_RUNTIME_EXPORT cl_int (*clGetMemObjectInfo)(cl_mem, cl_mem_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clGetImageInfo)(cl_mem, cl_image_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clSetMemObjectDestructorCallback)(cl_mem, void (CL_CALLBACK*) (cl_mem, void*), void*);
extern CL_RUNTIME_EXPORT cl_sampler (*clCreateSampler)(cl_context, cl_bool, cl_addressing_mode, cl_filter_mode, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clRetainSampler)(cl_sampler);
extern CL_RUNTIME_EXPORT cl_int (*clReleaseSampler)(cl_sampler);
extern CL_RUNTIME_EXPORT cl_int (*clGetSamplerInfo)(cl_sampler, cl_sampler_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_program (*clCreateProgramWithSource)(cl_context, cl_uint, const char**, const size_t*, cl_int*);
extern CL_RUNTIME_EXPORT cl_program (*clCreateProgramWithBinary)(cl_context, cl_uint, const cl_device_id*, const size_t*, const unsigned char**, cl_int*, cl_int*);
extern CL_RUNTIME_EXPORT cl_program (*clCreateProgramWithBuiltInKernels)(cl_context, cl_uint, const cl_device_id*, const char*, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clRetainProgram)(cl_program);
extern CL_RUNTIME_EXPORT cl_int (*clReleaseProgram)(cl_program);
extern CL_RUNTIME_EXPORT cl_int (*clBuildProgram)(cl_program, cl_uint, const cl_device_id*, const char*, void (CL_CALLBACK*) (cl_program, void*), void*);
extern CL_RUNTIME_EXPORT cl_int (*clCompileProgram)(cl_program, cl_uint, const cl_device_id*, const char*, cl_uint, const cl_program*, const char**, void (CL_CALLBACK*) (cl_program, void*), void*);
extern CL_RUNTIME_EXPORT cl_program (*clLinkProgram)(cl_context, cl_uint, const cl_device_id*, const char*, cl_uint, const cl_program*, void (CL_CALLBACK*) (cl_program, void*), void*, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clUnloadPlatformCompiler)(cl_platform_id);
extern CL_RUNTIME_EXPORT cl_int (*clGetProgramInfo)(cl_program, cl_program_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clGetProgramBuildInfo)(cl_program, cl_device_id, cl_program_build_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_kernel (*clCreateKernel)(cl_program, const char*, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clCreateKernelsInProgram)(cl_program, cl_uint, cl_kernel*, cl_uint*);
extern CL_RUNTIME_EXPORT cl_int (*clRetainKernel)(cl_kernel);
extern CL_RUNTIME_EXPORT cl_int (*clReleaseKernel)(cl_kernel);
extern CL_RUNTIME_EXPORT cl_int (*clSetKernelArg)(cl_kernel, cl_uint, size_t, const void*);
extern CL_RUNTIME_EXPORT cl_int (*clGetKernelInfo)(cl_kernel, cl_kernel_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clGetKernelArgInfo)(cl_kernel, cl_uint, cl_kernel_arg_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clGetKernelWorkGroupInfo)(cl_kernel, cl_device_id, cl_kernel_work_group_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clWaitForEvents)(cl_uint, const cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clGetEventInfo)(cl_event, cl_event_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_event (*clCreateUserEvent)(cl_context, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clRetainEvent)(cl_event);
extern CL_RUNTIME_EXPORT cl_int (*clReleaseEvent)(cl_event);
extern CL_RUNTIME_EXPORT cl_int (*clSetUserEventStatus)(cl_event, cl_int);
extern CL_RUNTIME_EXPORT cl_int (*clSetEventCallback)(cl_event, cl_int, void (CL_CALLBACK*) (cl_event, cl_int, void*), void*);
extern CL_RUNTIME_EXPORT cl_int (*clGetEventProfilingInfo)(cl_event, cl_profiling_info, size_t, void*, size_t*);
extern CL_RUNTIME_EXPORT cl_int (*clFlush)(cl_command_queue);
extern CL_RUNTIME_EXPORT cl_int (*clFinish)(cl_command_queue);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueReadBuffer)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueReadBufferRect)(cl_command_queue, cl_mem, cl_bool, const size_t*, const size_t*, const size_t*, size_t, size_t, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueWriteBuffer)(cl_command_queue, cl_mem, cl_bool, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueWriteBufferRect)(cl_command_queue, cl_mem, cl_bool, const size_t*, const size_t*, const size_t*, size_t, size_t, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueFillBuffer)(cl_command_queue, cl_mem, const void*, size_t, size_t, size_t, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueCopyBuffer)(cl_command_queue, cl_mem, cl_mem, size_t, size_t, size_t, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueCopyBufferRect)(cl_command_queue, cl_mem, cl_mem, const size_t*, const size_t*, const size_t*, size_t, size_t, size_t, size_t, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueReadImage)(cl_command_queue, cl_mem, cl_bool, const size_t*, const size_t*, size_t, size_t, void*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueWriteImage)(cl_command_queue, cl_mem, cl_bool, const size_t*, const size_t*, size_t, size_t, const void*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueFillImage)(cl_command_queue, cl_mem, const void*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueCopyImage)(cl_command_queue, cl_mem, cl_mem, const size_t*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueCopyImageToBuffer)(cl_command_queue, cl_mem, cl_mem, const size_t*, const size_t*, size_t, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueCopyBufferToImage)(cl_command_queue, cl_mem, cl_mem, size_t, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT void* (*clEnqueueMapBuffer)(cl_command_queue, cl_mem, cl_bool, cl_map_flags, size_t, size_t, cl_uint, const cl_event*, cl_event*, cl_int*);
extern CL_RUNTIME_EXPORT void* (*clEnqueueMapImage)(cl_command_queue, cl_mem, cl_bool, cl_map_flags, const size_t*, const size_t*, size_t*, size_t*, cl_uint, const cl_event*, cl_event*, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueUnmapMemObject)(cl_command_queue, cl_mem, void*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueMigrateMemObjects)(cl_command_queue, cl_uint, const cl_mem*, cl_mem_migration_flags, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueNDRangeKernel)(cl_command_queue, cl_kernel, cl_uint, const size_t*, const size_t*, const size_t*, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueTask)(cl_command_queue, cl_kernel, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueNativeKernel)(cl_command_queue, void (CL_CALLBACK*) (void*), void*, size_t, cl_uint, const cl_mem*, const void**, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueMarkerWithWaitList)(cl_command_queue, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueBarrierWithWaitList)(cl_command_queue, cl_uint, const cl_event*, cl_event*);
extern CL_RUNTIME_EXPORT void* (*clGetExtensionFunctionAddressForPlatform)(cl_platform_id, const char*);
extern CL_RUNTIME_EXPORT cl_mem (*clCreateImage2D)(cl_context, cl_mem_flags, const cl_image_format*, size_t, size_t, size_t, void*, cl_int*);
extern CL_RUNTIME_EXPORT cl_mem (*clCreateImage3D)(cl_context, cl_mem_flags, const cl_image_format*, size_t, size_t, size_t, size_t, size_t, void*, cl_int*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueMarker)(cl_command_queue, cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueWaitForEvents)(cl_command_queue, cl_uint, const cl_event*);
extern CL_RUNTIME_EXPORT cl_int (*clEnqueueBarrier)(cl_command_queue);
extern CL_RUNTIME_EXPORT cl_int (*clUnloadCompiler)();
extern CL_RUNTIME_EXPORT void* (*clGetExtensionFunctionAddress)(const char*);

#endif

#endif

#endif // __OPENCV_OCL_CL_RUNTIME_OPENCL_HPP__
