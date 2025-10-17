function(buildExample EXAMPLE_NAME)
	SET(EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME})
	message(STATUS "Generating project file for example in ${EXAMPLE_FOLDER}")
	# Main
	file(GLOB SOURCE *.cpp ${BASE_HEADERS} ${EXAMPLE_FOLDER}/*.cpp)
    SET(MAIN_HEADER ${EXAMPLE_FOLDER}/${EXAMPLE_NAME}.hpp)
	SET(MAIN_CPP ${EXAMPLE_FOLDER}/${EXAMPLE_NAME}.cpp)
	if(EXISTS ${EXAMPLE_FOLDER}/main.cpp)
		SET(MAIN_CPP ${EXAMPLE_FOLDER}/main.cpp)
	ENDIF()
	if(EXISTS ${EXAMPLE_FOLDER}/${EXAMPLE_NAME}.h)
		SET(MAIN_HEADER ${EXAMPLE_FOLDER}/${EXAMPLE_NAME}.h)
	ENDIF()

	# Add shaders
    set(SHADER_DIR "${EXAMPLE_FOLDER}/Shaders")
    file(GLOB SHADERS "${SHADER_DIR}/*.hlsl" "${SHADER_DIR}/*.vert" "${SHADER_DIR}/*.frag" "${SHADER_DIR}/*.comp" "${SHADER_DIR}/*.geom" "${SHADER_DIR}/*.tesc" "${SHADER_DIR}/*.tese" "${SHADER_DIR}/*.mesh" "${SHADER_DIR}/*.task" "${SHADER_DIR}/*.rgen" "${SHADER_DIR}/*.rchit" "${SHADER_DIR}/*.rmiss" "${SHADER_DIR}/*.rcall" "${SHADER_DIR}/*.rahit" "${SHADER_DIR}/*.rint")
	source_group("Shaders/" FILES ${SHADERS})

	set_source_files_properties(${SHADERS} PROPERTIES VS_TOOL_OVERRIDE "none")

	# Add optional readme / tutorial
	file(GLOB README_FILES "${EXAMPLE_FOLDER}/*.md")

    add_executable(${EXAMPLE_NAME} ${MAIN_CPP} ${SOURCE} ${MAIN_HEADER} ${SHADERS} ${README_FILES})
	target_include_directories(${EXAMPLE_NAME} PUBLIC ${Vulkan_INCLUDE_DIRS})
    target_link_libraries(${EXAMPLE_NAME} PUBLIC ${Vulkan_LIBRARIES} ImaginaryMathLibrary)

	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
	set_target_properties(${EXAMPLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
endfunction(buildExample)

# Build all examples
function(buildExamples)
	foreach(EXAMPLE ${EXAMPLES})
		buildExample(${EXAMPLE})
	endforeach(EXAMPLE)
endfunction(buildExamples)

set(EXAMPLES
	pbr
	#computecloth
	#computecullandlod
	#computeheadless
	#computenbody
	#computeparticles
	#computeraytracing
	#computeshader
	#conditionalrender
	#conservativeraster
	#debugprintf
	#debugutils
	#deferred
	#deferredmultisampling
	#deferredshadows
	#descriptorbuffer
	#descriptorindexing
	#descriptorsets
	#displacement
	#distancefieldfonts
	#dynamicrendering
	#dynamicstate
	#dynamicuniformbuffer	
	#gears
	#geometryshader
	#gltfloading
	#gltfscenerendering
	#gltfskinning
	#graphicspipelinelibrary
	#hdr
	#imgui
	#indirectdraw
	#inlineuniformblocks
	#inputattachments
	#instancing
	#meshshader
	#multisampling
	#multithreading
	#multiview
	#negativeviewportheight	
	#occlusionquery
	#offscreen
	#oit
	#parallaxmapping
	#particlesystem
	#pbrbasic
	#pbribl
	#pbrtexture
	#pipelines
	#pipelinestatistics
	#pushconstants
	#pushdescriptors
	#radialblur
	#rayquery
	#raytracingbasic
	#raytracingcallable
	#raytracinggltf
	#raytracingintersection
	#raytracingreflections
	#raytracingsbtdata
	#raytracingshadows
	#raytracingtextures
	#renderheadless
	#screenshot
	#shaderobjects
	#shadowmapping
	#shadowmappingomni
	#shadowmappingcascade
	#specializationconstants
	#sphericalenvmapping
	#ssao
	#stencilbuffer
	#subpasses
	#terraintessellation
	#tessellation
	#textoverlay
	#texture
	#texture3d
	#texturearray
	#texturecubemap
	#texturecubemaparray
	#texturemipmapgen
	#texturesparseresidency
	#triangle
	#variablerateshading
	#vertexattributes
	#viewportarray
	#vulkanscene
)

buildExamples()