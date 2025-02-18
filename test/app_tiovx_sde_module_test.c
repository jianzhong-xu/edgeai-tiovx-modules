/*
 *
 * Copyright (c) 2021 Texas Instruments Incorporated
 *
 * All rights reserved not granted herein.
 *
 * Limited License.
 *
 * Texas Instruments Incorporated grants a world-wide, royalty-free, non-exclusive
 * license under copyrights and patents it now or hereafter owns or controls to make,
 * have made, use, import, offer to sell and sell ("Utilize") this software subject to the
 * terms herein.  With respect to the foregoing patent license, such license is granted
 * solely to the extent that any such patent is necessary to Utilize the software alone.
 * The patent license shall not apply to any combinations which include this software,
 * other than combinations with devices manufactured by or for TI ("TI Devices").
 * No hardware patent is licensed hereunder.
 *
 * Redistributions must preserve existing copyright notices and reproduce this license
 * (including the above copyright notice and the disclaimer and (if applicable) source
 * code license limitations below) in the documentation and/or other materials provided
 * with the distribution
 *
 * Redistribution and use in binary form, without modification, are permitted provided
 * that the following conditions are met:
 *
 * *       No reverse engineering, decompilation, or disassembly of this software is
 * permitted with respect to any software provided in binary form.
 *
 * *       any redistribution and use are licensed by TI for use only with TI Devices.
 *
 * *       Nothing shall obligate TI to provide you with source code for the software
 * licensed and provided to you in object code.
 *
 * If software source code is provided to you, modification and redistribution of the
 * source code are permitted provided that the following conditions are met:
 *
 * *       any redistribution and use of the source code, including any resulting derivative
 * works, are licensed by TI for use only with TI Devices.
 *
 * *       any redistribution and use of any object code compiled from the source code
 * and any resulting derivative works, are licensed by TI for use only with TI Devices.
 *
 * Neither the name of Texas Instruments Incorporated nor the names of its suppliers
 *
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * DISCLAIMER.
 *
 * THIS SOFTWARE IS PROVIDED BY TI AND TI'S LICENSORS "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL TI AND TI'S LICENSORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include <tiovx_utils.h>
#include "tiovx_sde_module.h"

#define APP_BUFQ_DEPTH   (1)
#define APP_NUM_CH       (1)

#define IMAGE_WIDTH  (1280)
#define IMAGE_HEIGHT (720)

typedef struct {

    /* OpenVX references */
    vx_context context;

    vx_graph   graph;

    TIOVXSdeModuleObj  sdeObj;

} AppObj;

AppObj gAppObj;

static vx_status app_init(AppObj *obj);
static void app_deinit(AppObj *obj);
static vx_status app_create_graph(AppObj *obj);
static vx_status app_verify_graph(AppObj *obj);
static vx_status app_run_graph(AppObj *obj);
static void app_delete_graph(AppObj *obj);

vx_status app_modules_sde_test(vx_int32 argc, vx_char* argv[])
{
    AppObj *obj = &gAppObj;
    vx_status status = VX_SUCCESS;

    status = app_init(obj);
    APP_PRINTF("App Init Done! \n");

    if(status == VX_SUCCESS)
    {
        status = app_create_graph(obj);
        APP_PRINTF("App Create Graph Done! \n");
    }
    if(status == VX_SUCCESS)
    {
        status = app_verify_graph(obj);
        APP_PRINTF("App Verify Graph Done! \n");
    }
    if (status == VX_SUCCESS)
    {
        status = app_run_graph(obj);
        APP_PRINTF("App Run Graph Done! \n");
    }

    app_delete_graph(obj);
    APP_PRINTF("App Delete Graph Done! \n");

    app_deinit(obj);
    APP_PRINTF("App De-init Done! \n");

    return status;
}

static vx_status app_init(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    /* Create OpenVx Context */
    obj->context = vxCreateContext();
    status = vxGetStatus((vx_reference) obj->context);

    if(status == VX_SUCCESS)
    {
        tivxHwaLoadKernels(obj->context);
    }

    if(status == VX_SUCCESS)
    {
        TIOVXSdeModuleObj *sdeObj = &obj->sdeObj;

        sdeObj->input_left.bufq_depth = APP_BUFQ_DEPTH;
        sdeObj->input_left.color_format = VX_DF_IMAGE_NV12;

        sdeObj->input_right.bufq_depth = APP_BUFQ_DEPTH;
        sdeObj->input_right.color_format = VX_DF_IMAGE_NV12;

        sdeObj->output.bufq_depth = APP_BUFQ_DEPTH;
        sdeObj->output.color_format = VX_DF_IMAGE_S16;

        sdeObj->num_channels = APP_NUM_CH;
        sdeObj->width = IMAGE_WIDTH;
        sdeObj->height = IMAGE_HEIGHT;

        //Configure sde params
        sdeObj->params.median_filter_enable        = 0;
        sdeObj->params.reduced_range_search_enable = 0;
        sdeObj->params.disparity_min               = 0;
        sdeObj->params.disparity_max               = 1;
        sdeObj->params.threshold_left_right        = 0;
        sdeObj->params.texture_filter_enable       = 0;
        sdeObj->params.threshold_texture           = 0;
        sdeObj->params.aggregation_penalty_p1      = 0;
        sdeObj->params.aggregation_penalty_p2      = 0;
        sdeObj->params.confidence_score_map[0]     = 0;
        sdeObj->params.confidence_score_map[1]     = 4;
        sdeObj->params.confidence_score_map[2]     = 9;
        sdeObj->params.confidence_score_map[3]     = 18;
        sdeObj->params.confidence_score_map[4]     = 28;
        sdeObj->params.confidence_score_map[5]     = 43;
        sdeObj->params.confidence_score_map[6]     = 109;
        sdeObj->params.confidence_score_map[7]     = 127;

        /* Initialize modules */
        status = tiovx_sde_module_init(obj->context, sdeObj);
        APP_PRINTF("Sde Init Done! \n");
    }

    return status;
}

static void app_deinit(AppObj *obj)
{
    tiovx_sde_module_deinit(&obj->sdeObj);

    tivxHwaUnLoadKernels(obj->context);

    vxReleaseContext(&obj->context);
}

static void app_delete_graph(AppObj *obj)
{
    tiovx_sde_module_delete(&obj->sdeObj);

    vxReleaseGraph(&obj->graph);
}

static vx_status app_create_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    vx_graph_parameter_queue_params_t graph_parameters_queue_params_list[8];
    vx_int32 graph_parameter_index;

    obj->graph = vxCreateGraph(obj->context);
    status = vxGetStatus((vx_reference)obj->graph);

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_sde_module_create(obj->graph, &obj->sdeObj, NULL, NULL, TIVX_TARGET_DMPAC_SDE);
    }

    graph_parameter_index = 0;
    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->sdeObj.node, 1);
        obj->sdeObj.input_left.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->sdeObj.input_left.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->sdeObj.node, 2);
        obj->sdeObj.input_right.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->sdeObj.input_right.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = add_graph_parameter_by_node_index(obj->graph, obj->sdeObj.node, 3);
        obj->sdeObj.output.graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].graph_parameter_index = graph_parameter_index;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list_size = APP_BUFQ_DEPTH;
        graph_parameters_queue_params_list[graph_parameter_index].refs_list = (vx_reference*)&obj->sdeObj.output.image_handle[0];
        graph_parameter_index++;
    }

    if((vx_status)VX_SUCCESS == status)
    {
        status = vxSetGraphScheduleConfig(obj->graph,
                    VX_GRAPH_SCHEDULE_MODE_QUEUE_MANUAL,
                    graph_parameter_index,
                    graph_parameters_queue_params_list);
    }

    return status;
}

static vx_status app_verify_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    status = vxVerifyGraph(obj->graph);

    APP_PRINTF("App Verify Graph Done!\n");

    if((vx_status)VX_SUCCESS == status)
    {
        status = tiovx_sde_module_release_buffers(&obj->sdeObj);
    }

    return status;
}

static vx_status app_run_graph(AppObj *obj)
{
    vx_status status = VX_SUCCESS;

    char * input_left_filename = "/opt/edgeai-tiovx-modules/data/input/avp3_1280x720_0_nv12.yuv";
    char * input_right_filename = "/opt/edgeai-tiovx-modules/data/input/avp3_1280x720_1_nv12.yuv";
    char * output_filename = "/opt/edgeai-tiovx-modules/data/output/output_sde_s16.raw";

    vx_image input_left_o, input_right_o;
    vx_image output_o;

    TIOVXSdeModuleObj *sdeObj = &obj->sdeObj;
    vx_int32 bufq;
    uint32_t num_refs;

    void *inLeftAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *inRightAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};
    void *outAddr[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES] = {NULL};

    vx_uint32 inLeftSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 inRightSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];
    vx_uint32 outSizes[APP_BUFQ_DEPTH][TIOVX_MODULES_MAX_REF_HANDLES];

    /* These can be moved to app_init() */
    allocate_image_buffers(&sdeObj->input_left, inLeftAddr, inLeftSizes);
    allocate_image_buffers(&sdeObj->input_right, inRightAddr, inRightSizes);
    allocate_image_buffers(&sdeObj->output, outAddr, outSizes);

    bufq = 0;
    assign_image_buffers(&sdeObj->input_left, inLeftAddr[bufq], inLeftSizes[bufq], bufq);
    assign_image_buffers(&sdeObj->input_right, inRightAddr[bufq], inRightSizes[bufq], bufq);
    assign_image_buffers(&sdeObj->output, outAddr[bufq], outSizes[bufq], bufq);

    readImage(input_left_filename, sdeObj->input_left.image_handle[0]);
    readImage(input_right_filename, sdeObj->input_right.image_handle[0]);

    APP_PRINTF("Enqueueing input buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 0, (vx_reference*)&sdeObj->input_left.image_handle[0], 1);
    vxGraphParameterEnqueueReadyRef(obj->graph, 1, (vx_reference*)&sdeObj->input_right.image_handle[0], 1);
    APP_PRINTF("Enqueueing output buffers!\n");
    vxGraphParameterEnqueueReadyRef(obj->graph, 2, (vx_reference*)&sdeObj->output.image_handle[0], 1);

    APP_PRINTF("Processing!\n");
    status = vxScheduleGraph(obj->graph);
    if((vx_status)VX_SUCCESS != status) {
      APP_PRINTF("Schedule Graph failed: %d!\n", status);
    }
    status = vxWaitGraph(obj->graph);
    if((vx_status)VX_SUCCESS != status) {
      APP_PRINTF("Wait Graph failed: %d!\n", status);
    }

    vxGraphParameterDequeueDoneRef(obj->graph, 0, (vx_reference*)&input_left_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, 1, (vx_reference*)&input_right_o, 1, &num_refs);
    vxGraphParameterDequeueDoneRef(obj->graph, 2, (vx_reference*)&output_o, 1, &num_refs);

    writeImage(output_filename, sdeObj->output.image_handle[0]);

    release_image_buffers(&sdeObj->input_left, inLeftAddr[bufq], inLeftSizes[bufq], bufq);
    release_image_buffers(&sdeObj->input_right, inRightAddr[bufq], inRightSizes[bufq], bufq);
    release_image_buffers(&sdeObj->output, outAddr[bufq], outSizes[bufq], bufq);

    /* These can move to deinit() */
    delete_image_buffers(&sdeObj->input_left, inLeftAddr, inLeftSizes);
    delete_image_buffers(&sdeObj->input_right, inRightAddr, inRightSizes);
    delete_image_buffers(&sdeObj->output, outAddr, outSizes);

    return status;
}

