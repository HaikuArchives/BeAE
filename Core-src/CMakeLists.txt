#Determine whether we are building outside of the main source directory
if (NOT ${BEAE_SOURCE_DIR} STREQUAL ${BEAE_BINARY_DIR})
	set(OUT_OF_SOURCE 1)
endif(NOT ${BEAE_SOURCE_DIR} STREQUAL ${BEAE_BINARY_DIR})


#This can be changed to something else like 'generated'
#It could even be set to output to the BeAE directory which
#would remove the need to copy bitmaps for in source builds.
set(BEAE_BUILD_DIR ${BEAE_BINARY_DIR}/build)


set(BEAE_SRCS AboutBox.cpp AmplifierFilter.cpp AnalyzeWindow.cpp BassBoostFilter.cpp
				BigView.cpp BitmapDrawer.cpp CommonClass.cpp CommonPool.cpp CompressorFilter.cpp
				DelayWindow.cpp Filters.cpp FreqWindow.cpp History.cpp IndexView.cpp LimiterFilter.cpp
				MainWindow.cpp MyClipBoard.cpp MyStringItem.cpp NormalizeFilter.cpp OpenPanel.cpp
				PeakFile.cpp PointersView.cpp PrefColors.cpp PrefGeneral.cpp PrefKeys.cpp PrefView.cpp
				PrefWindow.cpp Preferences.cpp ProgressWindow.cpp RealtimeFilter.cpp ResampleWindow.cpp
				ReverbWindow.cpp RoomFilter.cpp SampleScopeWindow.cpp SampleView.cpp SavePanel.cpp
				Shortcut.cpp SpectrumWindow.cpp SpinButton.cpp SpinControl.cpp SpinSlider.cpp
				SwatchView.cpp TimeBarView.cpp ToolBarView.cpp ToolTip.cpp ToolTipWindow.cpp
				TransportView.cpp VMSystem.cpp VUView.cpp ValuesView.cpp YLanguageClass.cpp
				YPreferences.cpp main.cpp)


add_executable(BeAE ${BEAE_SRCS})


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-multichar -Woverloaded-virtual -Wno-unknown-pragmas -Wno-ctor-dtor-privacy")


target_link_libraries(BeAE "be" "root" "tracker" "media" "translation" "game")


#Change the default location of our output
set_property(TARGET BeAE PROPERTY RUNTIME_OUTPUT_DIRECTORY ${BEAE_BUILD_DIR})


haiku_add_resource_def(BeAE BeAE.rdef)


haiku_auto_mimetype(BeAE)


#Copy the bitmaps to the build directory so that BeAE will run properly.
#Could use several improvements!
add_custom_command(TARGET BeAE POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/BeAE/Bitmaps ${BEAE_BUILD_DIR}/Bitmaps
					COMMENT "Copying bitmaps to build directory")

#Copy the languages to the build directory so that BeAE will run properly.
add_custom_command(TARGET BeAE POST_BUILD
					COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/BeAE/Languages ${BEAE_BUILD_DIR}/Languages
					COMMENT "Copying languages to build directory")

if(OUT_OF_SOURCE)

	add_custom_target(distro
					COMMAND ${CMAKE_COMMAND} -E copy_directory ${BEAE_SOURCE_DIR}/BeAE ${BEAE_BINARY_DIR}/BeAE
					COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BEAE_BUILD_DIR}/BeAE ${BEAE_BINARY_DIR}/BeAE/BeAE
					#COMMAND execute zip or cpack command here?
					DEPENDS BeAE
					COMMENT "Creating BeAE distribution")

else(OUT_OF_SOURCE)

	add_custom_target(distro
					COMMAND ${CMAKE_COMMAND} -E copy_if_different ${BEAE_BUILD_DIR}/BeAE ${BEAE_SOURCE_DIR}/BeAE/BeAE
					#COMMAND execute zip or cpack command here?
					DEPENDS BeAE
					COMMENT "Creating BeAE distribution")

endif(OUT_OF_SOURCE)
