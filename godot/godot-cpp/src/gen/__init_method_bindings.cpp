#include <GlobalConstants.hpp>
#include <ARVRAnchor.hpp>
#include <ARVRCamera.hpp>
#include <ARVRController.hpp>
#include <ARVRInterface.hpp>
#include <ARVRInterfaceGDNative.hpp>
#include <ARVROrigin.hpp>
#include <ARVRPositionalTracker.hpp>
#include <ARVRServer.hpp>
#include <AStar.hpp>
#include <AStar2D.hpp>
#include <AcceptDialog.hpp>
#include <AnimatedSprite.hpp>
#include <AnimatedSprite3D.hpp>
#include <AnimatedTexture.hpp>
#include <Animation.hpp>
#include <AnimationNode.hpp>
#include <AnimationNodeAdd2.hpp>
#include <AnimationNodeAdd3.hpp>
#include <AnimationNodeAnimation.hpp>
#include <AnimationNodeBlend2.hpp>
#include <AnimationNodeBlend3.hpp>
#include <AnimationNodeBlendSpace1D.hpp>
#include <AnimationNodeBlendSpace2D.hpp>
#include <AnimationNodeBlendTree.hpp>
#include <AnimationNodeOneShot.hpp>
#include <AnimationNodeOutput.hpp>
#include <AnimationNodeStateMachine.hpp>
#include <AnimationNodeStateMachinePlayback.hpp>
#include <AnimationNodeStateMachineTransition.hpp>
#include <AnimationNodeTimeScale.hpp>
#include <AnimationNodeTimeSeek.hpp>
#include <AnimationNodeTransition.hpp>
#include <AnimationPlayer.hpp>
#include <AnimationRootNode.hpp>
#include <AnimationTrackEditPlugin.hpp>
#include <AnimationTree.hpp>
#include <AnimationTreePlayer.hpp>
#include <Area.hpp>
#include <Area2D.hpp>
#include <ArrayMesh.hpp>
#include <AtlasTexture.hpp>
#include <AudioBusLayout.hpp>
#include <AudioEffect.hpp>
#include <AudioEffectAmplify.hpp>
#include <AudioEffectBandLimitFilter.hpp>
#include <AudioEffectBandPassFilter.hpp>
#include <AudioEffectChorus.hpp>
#include <AudioEffectCompressor.hpp>
#include <AudioEffectDelay.hpp>
#include <AudioEffectDistortion.hpp>
#include <AudioEffectEQ.hpp>
#include <AudioEffectEQ10.hpp>
#include <AudioEffectEQ21.hpp>
#include <AudioEffectEQ6.hpp>
#include <AudioEffectFilter.hpp>
#include <AudioEffectHighPassFilter.hpp>
#include <AudioEffectHighShelfFilter.hpp>
#include <AudioEffectInstance.hpp>
#include <AudioEffectLimiter.hpp>
#include <AudioEffectLowPassFilter.hpp>
#include <AudioEffectLowShelfFilter.hpp>
#include <AudioEffectNotchFilter.hpp>
#include <AudioEffectPanner.hpp>
#include <AudioEffectPhaser.hpp>
#include <AudioEffectPitchShift.hpp>
#include <AudioEffectRecord.hpp>
#include <AudioEffectReverb.hpp>
#include <AudioEffectSpectrumAnalyzer.hpp>
#include <AudioEffectSpectrumAnalyzerInstance.hpp>
#include <AudioEffectStereoEnhance.hpp>
#include <AudioServer.hpp>
#include <AudioStream.hpp>
#include <AudioStreamGenerator.hpp>
#include <AudioStreamGeneratorPlayback.hpp>
#include <AudioStreamMicrophone.hpp>
#include <AudioStreamOGGVorbis.hpp>
#include <AudioStreamPlayback.hpp>
#include <AudioStreamPlaybackResampled.hpp>
#include <AudioStreamPlayer.hpp>
#include <AudioStreamPlayer2D.hpp>
#include <AudioStreamPlayer3D.hpp>
#include <AudioStreamRandomPitch.hpp>
#include <AudioStreamSample.hpp>
#include <BackBufferCopy.hpp>
#include <BakedLightmap.hpp>
#include <BakedLightmapData.hpp>
#include <BaseButton.hpp>
#include <BitMap.hpp>
#include <BitmapFont.hpp>
#include <Bone2D.hpp>
#include <BoneAttachment.hpp>
#include <BoxContainer.hpp>
#include <BoxShape.hpp>
#include <BulletPhysicsDirectBodyState.hpp>
#include <BulletPhysicsServer.hpp>
#include <Button.hpp>
#include <ButtonGroup.hpp>
#include <CPUParticles.hpp>
#include <CPUParticles2D.hpp>
#include <CSGBox.hpp>
#include <CSGCombiner.hpp>
#include <CSGCylinder.hpp>
#include <CSGMesh.hpp>
#include <CSGPolygon.hpp>
#include <CSGPrimitive.hpp>
#include <CSGShape.hpp>
#include <CSGSphere.hpp>
#include <CSGTorus.hpp>
#include <Camera.hpp>
#include <Camera2D.hpp>
#include <CameraFeed.hpp>
#include <CameraServer.hpp>
#include <CameraTexture.hpp>
#include <CanvasItem.hpp>
#include <CanvasItemMaterial.hpp>
#include <CanvasLayer.hpp>
#include <CanvasModulate.hpp>
#include <CapsuleMesh.hpp>
#include <CapsuleShape.hpp>
#include <CapsuleShape2D.hpp>
#include <CenterContainer.hpp>
#include <CharFXTransform.hpp>
#include <CheckBox.hpp>
#include <CheckButton.hpp>
#include <CircleShape2D.hpp>
#include <ClippedCamera.hpp>
#include <CollisionObject.hpp>
#include <CollisionObject2D.hpp>
#include <CollisionPolygon.hpp>
#include <CollisionPolygon2D.hpp>
#include <CollisionShape.hpp>
#include <CollisionShape2D.hpp>
#include <ColorPicker.hpp>
#include <ColorPickerButton.hpp>
#include <ColorRect.hpp>
#include <ConcavePolygonShape.hpp>
#include <ConcavePolygonShape2D.hpp>
#include <ConeTwistJoint.hpp>
#include <ConfigFile.hpp>
#include <ConfirmationDialog.hpp>
#include <Container.hpp>
#include <Control.hpp>
#include <ConvexPolygonShape.hpp>
#include <ConvexPolygonShape2D.hpp>
#include <Crypto.hpp>
#include <CryptoKey.hpp>
#include <CubeMap.hpp>
#include <CubeMesh.hpp>
#include <Curve.hpp>
#include <Curve2D.hpp>
#include <Curve3D.hpp>
#include <CurveTexture.hpp>
#include <CylinderMesh.hpp>
#include <CylinderShape.hpp>
#include <DampedSpringJoint2D.hpp>
#include <DirectionalLight.hpp>
#include <DynamicFont.hpp>
#include <DynamicFontData.hpp>
#include <EditorExportPlugin.hpp>
#include <EditorFeatureProfile.hpp>
#include <EditorFileDialog.hpp>
#include <EditorFileSystem.hpp>
#include <EditorFileSystemDirectory.hpp>
#include <EditorImportPlugin.hpp>
#include <EditorInspector.hpp>
#include <EditorInspectorPlugin.hpp>
#include <EditorInterface.hpp>
#include <EditorNavigationMeshGenerator.hpp>
#include <EditorPlugin.hpp>
#include <EditorProperty.hpp>
#include <EditorResourceConversionPlugin.hpp>
#include <EditorResourcePreview.hpp>
#include <EditorResourcePreviewGenerator.hpp>
#include <EditorSceneImporter.hpp>
#include <EditorSceneImporterAssimp.hpp>
#include <EditorScenePostImport.hpp>
#include <EditorScript.hpp>
#include <EditorSelection.hpp>
#include <EditorSettings.hpp>
#include <EditorSpatialGizmo.hpp>
#include <EditorSpatialGizmoPlugin.hpp>
#include <EditorSpinSlider.hpp>
#include <EditorVCSInterface.hpp>
#include <EncodedObjectAsID.hpp>
#include <Environment.hpp>
#include <Expression.hpp>
#include <FileDialog.hpp>
#include <Font.hpp>
#include <FuncRef.hpp>
#include <GDNative.hpp>
#include <GDNativeLibrary.hpp>
#include <GDScript.hpp>
#include <GDScriptFunctionState.hpp>
#include <GIProbe.hpp>
#include <GIProbeData.hpp>
#include <Generic6DOFJoint.hpp>
#include <GeometryInstance.hpp>
#include <Gradient.hpp>
#include <GradientTexture.hpp>
#include <GraphEdit.hpp>
#include <GraphNode.hpp>
#include <GridContainer.hpp>
#include <GridMap.hpp>
#include <GrooveJoint2D.hpp>
#include <HBoxContainer.hpp>
#include <HScrollBar.hpp>
#include <HSeparator.hpp>
#include <HSlider.hpp>
#include <HSplitContainer.hpp>
#include <HTTPClient.hpp>
#include <HTTPRequest.hpp>
#include <HashingContext.hpp>
#include <HeightMapShape.hpp>
#include <HingeJoint.hpp>
#include <IP.hpp>
#include <IP_Unix.hpp>
#include <Image.hpp>
#include <ImageTexture.hpp>
#include <ImmediateGeometry.hpp>
#include <Input.hpp>
#include <InputDefault.hpp>
#include <InputEvent.hpp>
#include <InputEventAction.hpp>
#include <InputEventGesture.hpp>
#include <InputEventJoypadButton.hpp>
#include <InputEventJoypadMotion.hpp>
#include <InputEventKey.hpp>
#include <InputEventMIDI.hpp>
#include <InputEventMagnifyGesture.hpp>
#include <InputEventMouse.hpp>
#include <InputEventMouseButton.hpp>
#include <InputEventMouseMotion.hpp>
#include <InputEventPanGesture.hpp>
#include <InputEventScreenDrag.hpp>
#include <InputEventScreenTouch.hpp>
#include <InputEventWithModifiers.hpp>
#include <InputMap.hpp>
#include <InstancePlaceholder.hpp>
#include <InterpolatedCamera.hpp>
#include <ItemList.hpp>
#include <JSONParseResult.hpp>
#include <JSONRPC.hpp>
#include <JavaClass.hpp>
#include <JavaClassWrapper.hpp>
#include <JavaScript.hpp>
#include <Joint.hpp>
#include <Joint2D.hpp>
#include <KinematicBody.hpp>
#include <KinematicBody2D.hpp>
#include <KinematicCollision.hpp>
#include <KinematicCollision2D.hpp>
#include <Label.hpp>
#include <LargeTexture.hpp>
#include <Light.hpp>
#include <Light2D.hpp>
#include <LightOccluder2D.hpp>
#include <Line2D.hpp>
#include <LineEdit.hpp>
#include <LineShape2D.hpp>
#include <LinkButton.hpp>
#include <Listener.hpp>
#include <MainLoop.hpp>
#include <MarginContainer.hpp>
#include <Material.hpp>
#include <MenuButton.hpp>
#include <Mesh.hpp>
#include <MeshDataTool.hpp>
#include <MeshInstance.hpp>
#include <MeshInstance2D.hpp>
#include <MeshLibrary.hpp>
#include <MeshTexture.hpp>
#include <MobileVRInterface.hpp>
#include <MultiMesh.hpp>
#include <MultiMeshInstance.hpp>
#include <MultiMeshInstance2D.hpp>
#include <MultiplayerAPI.hpp>
#include <MultiplayerPeerGDNative.hpp>
#include <NativeScript.hpp>
#include <Navigation.hpp>
#include <Navigation2D.hpp>
#include <NavigationMesh.hpp>
#include <NavigationMeshInstance.hpp>
#include <NavigationPolygon.hpp>
#include <NavigationPolygonInstance.hpp>
#include <NetworkedMultiplayerENet.hpp>
#include <NetworkedMultiplayerPeer.hpp>
#include <NinePatchRect.hpp>
#include <Node.hpp>
#include <Node2D.hpp>
#include <NoiseTexture.hpp>
#include <Object.hpp>
#include <OccluderPolygon2D.hpp>
#include <OmniLight.hpp>
#include <OpenSimplexNoise.hpp>
#include <OptionButton.hpp>
#include <PCKPacker.hpp>
#include <PHashTranslation.hpp>
#include <PackedDataContainer.hpp>
#include <PackedDataContainerRef.hpp>
#include <PackedScene.hpp>
#include <PacketPeer.hpp>
#include <PacketPeerGDNative.hpp>
#include <PacketPeerStream.hpp>
#include <PacketPeerUDP.hpp>
#include <Panel.hpp>
#include <PanelContainer.hpp>
#include <PanoramaSky.hpp>
#include <ParallaxBackground.hpp>
#include <ParallaxLayer.hpp>
#include <Particles.hpp>
#include <Particles2D.hpp>
#include <ParticlesMaterial.hpp>
#include <Path.hpp>
#include <Path2D.hpp>
#include <PathFollow.hpp>
#include <PathFollow2D.hpp>
#include <Performance.hpp>
#include <PhysicalBone.hpp>
#include <Physics2DDirectBodyState.hpp>
#include <Physics2DDirectBodyStateSW.hpp>
#include <Physics2DDirectSpaceState.hpp>
#include <Physics2DServer.hpp>
#include <Physics2DServerSW.hpp>
#include <Physics2DShapeQueryParameters.hpp>
#include <Physics2DShapeQueryResult.hpp>
#include <Physics2DTestMotionResult.hpp>
#include <PhysicsBody.hpp>
#include <PhysicsBody2D.hpp>
#include <PhysicsDirectBodyState.hpp>
#include <PhysicsDirectSpaceState.hpp>
#include <PhysicsMaterial.hpp>
#include <PhysicsServer.hpp>
#include <PhysicsShapeQueryParameters.hpp>
#include <PhysicsShapeQueryResult.hpp>
#include <PinJoint.hpp>
#include <PinJoint2D.hpp>
#include <PlaneMesh.hpp>
#include <PlaneShape.hpp>
#include <PluginScript.hpp>
#include <PointMesh.hpp>
#include <Polygon2D.hpp>
#include <PolygonPathFinder.hpp>
#include <Popup.hpp>
#include <PopupDialog.hpp>
#include <PopupMenu.hpp>
#include <PopupPanel.hpp>
#include <Position2D.hpp>
#include <Position3D.hpp>
#include <PrimitiveMesh.hpp>
#include <PrismMesh.hpp>
#include <ProceduralSky.hpp>
#include <ProgressBar.hpp>
#include <ProjectSettings.hpp>
#include <ProximityGroup.hpp>
#include <ProxyTexture.hpp>
#include <QuadMesh.hpp>
#include <RandomNumberGenerator.hpp>
#include <Range.hpp>
#include <RayCast.hpp>
#include <RayCast2D.hpp>
#include <RayShape.hpp>
#include <RayShape2D.hpp>
#include <RectangleShape2D.hpp>
#include <Reference.hpp>
#include <ReferenceRect.hpp>
#include <ReflectionProbe.hpp>
#include <RegEx.hpp>
#include <RegExMatch.hpp>
#include <RemoteTransform.hpp>
#include <RemoteTransform2D.hpp>
#include <Resource.hpp>
#include <ResourceFormatLoader.hpp>
#include <ResourceFormatLoaderCrypto.hpp>
#include <ResourceFormatSaver.hpp>
#include <ResourceFormatSaverCrypto.hpp>
#include <ResourceImporter.hpp>
#include <ResourceInteractiveLoader.hpp>
#include <ResourcePreloader.hpp>
#include <RichTextEffect.hpp>
#include <RichTextLabel.hpp>
#include <RigidBody.hpp>
#include <RigidBody2D.hpp>
#include <RootMotionView.hpp>
#include <SceneState.hpp>
#include <SceneTree.hpp>
#include <SceneTreeTimer.hpp>
#include <Script.hpp>
#include <ScriptCreateDialog.hpp>
#include <ScriptEditor.hpp>
#include <ScrollBar.hpp>
#include <ScrollContainer.hpp>
#include <SegmentShape2D.hpp>
#include <Separator.hpp>
#include <Shader.hpp>
#include <ShaderMaterial.hpp>
#include <Shape.hpp>
#include <Shape2D.hpp>
#include <ShortCut.hpp>
#include <Skeleton.hpp>
#include <Skeleton2D.hpp>
#include <SkeletonIK.hpp>
#include <Skin.hpp>
#include <SkinReference.hpp>
#include <Sky.hpp>
#include <Slider.hpp>
#include <SliderJoint.hpp>
#include <SoftBody.hpp>
#include <Spatial.hpp>
#include <SpatialGizmo.hpp>
#include <SpatialMaterial.hpp>
#include <SpatialVelocityTracker.hpp>
#include <SphereMesh.hpp>
#include <SphereShape.hpp>
#include <SpinBox.hpp>
#include <SplitContainer.hpp>
#include <SpotLight.hpp>
#include <SpringArm.hpp>
#include <Sprite.hpp>
#include <Sprite3D.hpp>
#include <SpriteBase3D.hpp>
#include <SpriteFrames.hpp>
#include <StaticBody.hpp>
#include <StaticBody2D.hpp>
#include <StreamPeer.hpp>
#include <StreamPeerBuffer.hpp>
#include <StreamPeerGDNative.hpp>
#include <StreamPeerSSL.hpp>
#include <StreamPeerTCP.hpp>
#include <StreamTexture.hpp>
#include <StyleBox.hpp>
#include <StyleBoxEmpty.hpp>
#include <StyleBoxFlat.hpp>
#include <StyleBoxLine.hpp>
#include <StyleBoxTexture.hpp>
#include <SurfaceTool.hpp>
#include <TCP_Server.hpp>
#include <TabContainer.hpp>
#include <Tabs.hpp>
#include <TextEdit.hpp>
#include <TextFile.hpp>
#include <Texture.hpp>
#include <Texture3D.hpp>
#include <TextureArray.hpp>
#include <TextureButton.hpp>
#include <TextureLayered.hpp>
#include <TextureProgress.hpp>
#include <TextureRect.hpp>
#include <Theme.hpp>
#include <TileMap.hpp>
#include <TileSet.hpp>
#include <Timer.hpp>
#include <ToolButton.hpp>
#include <TouchScreenButton.hpp>
#include <Translation.hpp>
#include <TranslationServer.hpp>
#include <Tree.hpp>
#include <TreeItem.hpp>
#include <TriangleMesh.hpp>
#include <Tween.hpp>
#include <UPNP.hpp>
#include <UPNPDevice.hpp>
#include <UndoRedo.hpp>
#include <VBoxContainer.hpp>
#include <VScrollBar.hpp>
#include <VSeparator.hpp>
#include <VSlider.hpp>
#include <VSplitContainer.hpp>
#include <VehicleBody.hpp>
#include <VehicleWheel.hpp>
#include <VideoPlayer.hpp>
#include <VideoStream.hpp>
#include <VideoStreamGDNative.hpp>
#include <VideoStreamTheora.hpp>
#include <VideoStreamWebm.hpp>
#include <Viewport.hpp>
#include <ViewportContainer.hpp>
#include <ViewportTexture.hpp>
#include <VisibilityEnabler.hpp>
#include <VisibilityEnabler2D.hpp>
#include <VisibilityNotifier.hpp>
#include <VisibilityNotifier2D.hpp>
#include <VisualInstance.hpp>
#include <VisualScript.hpp>
#include <VisualScriptBasicTypeConstant.hpp>
#include <VisualScriptBuiltinFunc.hpp>
#include <VisualScriptClassConstant.hpp>
#include <VisualScriptComment.hpp>
#include <VisualScriptComposeArray.hpp>
#include <VisualScriptCondition.hpp>
#include <VisualScriptConstant.hpp>
#include <VisualScriptConstructor.hpp>
#include <VisualScriptCustomNode.hpp>
#include <VisualScriptDeconstruct.hpp>
#include <VisualScriptEmitSignal.hpp>
#include <VisualScriptEngineSingleton.hpp>
#include <VisualScriptExpression.hpp>
#include <VisualScriptFunction.hpp>
#include <VisualScriptFunctionCall.hpp>
#include <VisualScriptFunctionState.hpp>
#include <VisualScriptGlobalConstant.hpp>
#include <VisualScriptIndexGet.hpp>
#include <VisualScriptIndexSet.hpp>
#include <VisualScriptInputAction.hpp>
#include <VisualScriptIterator.hpp>
#include <VisualScriptLists.hpp>
#include <VisualScriptLocalVar.hpp>
#include <VisualScriptLocalVarSet.hpp>
#include <VisualScriptMathConstant.hpp>
#include <VisualScriptNode.hpp>
#include <VisualScriptOperator.hpp>
#include <VisualScriptPreload.hpp>
#include <VisualScriptPropertyGet.hpp>
#include <VisualScriptPropertySet.hpp>
#include <VisualScriptResourcePath.hpp>
#include <VisualScriptReturn.hpp>
#include <VisualScriptSceneNode.hpp>
#include <VisualScriptSceneTree.hpp>
#include <VisualScriptSelect.hpp>
#include <VisualScriptSelf.hpp>
#include <VisualScriptSequence.hpp>
#include <VisualScriptSubCall.hpp>
#include <VisualScriptSwitch.hpp>
#include <VisualScriptTypeCast.hpp>
#include <VisualScriptVariableGet.hpp>
#include <VisualScriptVariableSet.hpp>
#include <VisualScriptWhile.hpp>
#include <VisualScriptYield.hpp>
#include <VisualScriptYieldSignal.hpp>
#include <VisualServer.hpp>
#include <VisualShader.hpp>
#include <VisualShaderNode.hpp>
#include <VisualShaderNodeBooleanConstant.hpp>
#include <VisualShaderNodeBooleanUniform.hpp>
#include <VisualShaderNodeColorConstant.hpp>
#include <VisualShaderNodeColorFunc.hpp>
#include <VisualShaderNodeColorOp.hpp>
#include <VisualShaderNodeColorUniform.hpp>
#include <VisualShaderNodeCompare.hpp>
#include <VisualShaderNodeCubeMap.hpp>
#include <VisualShaderNodeCubeMapUniform.hpp>
#include <VisualShaderNodeCustom.hpp>
#include <VisualShaderNodeDeterminant.hpp>
#include <VisualShaderNodeDotProduct.hpp>
#include <VisualShaderNodeExpression.hpp>
#include <VisualShaderNodeFaceForward.hpp>
#include <VisualShaderNodeFresnel.hpp>
#include <VisualShaderNodeGlobalExpression.hpp>
#include <VisualShaderNodeGroupBase.hpp>
#include <VisualShaderNodeIf.hpp>
#include <VisualShaderNodeInput.hpp>
#include <VisualShaderNodeIs.hpp>
#include <VisualShaderNodeOuterProduct.hpp>
#include <VisualShaderNodeOutput.hpp>
#include <VisualShaderNodeScalarClamp.hpp>
#include <VisualShaderNodeScalarConstant.hpp>
#include <VisualShaderNodeScalarDerivativeFunc.hpp>
#include <VisualShaderNodeScalarFunc.hpp>
#include <VisualShaderNodeScalarInterp.hpp>
#include <VisualShaderNodeScalarOp.hpp>
#include <VisualShaderNodeScalarSmoothStep.hpp>
#include <VisualShaderNodeScalarSwitch.hpp>
#include <VisualShaderNodeScalarUniform.hpp>
#include <VisualShaderNodeSwitch.hpp>
#include <VisualShaderNodeTexture.hpp>
#include <VisualShaderNodeTextureUniform.hpp>
#include <VisualShaderNodeTextureUniformTriplanar.hpp>
#include <VisualShaderNodeTransformCompose.hpp>
#include <VisualShaderNodeTransformConstant.hpp>
#include <VisualShaderNodeTransformDecompose.hpp>
#include <VisualShaderNodeTransformFunc.hpp>
#include <VisualShaderNodeTransformMult.hpp>
#include <VisualShaderNodeTransformUniform.hpp>
#include <VisualShaderNodeTransformVecMult.hpp>
#include <VisualShaderNodeUniform.hpp>
#include <VisualShaderNodeVec3Constant.hpp>
#include <VisualShaderNodeVec3Uniform.hpp>
#include <VisualShaderNodeVectorClamp.hpp>
#include <VisualShaderNodeVectorCompose.hpp>
#include <VisualShaderNodeVectorDecompose.hpp>
#include <VisualShaderNodeVectorDerivativeFunc.hpp>
#include <VisualShaderNodeVectorDistance.hpp>
#include <VisualShaderNodeVectorFunc.hpp>
#include <VisualShaderNodeVectorInterp.hpp>
#include <VisualShaderNodeVectorLen.hpp>
#include <VisualShaderNodeVectorOp.hpp>
#include <VisualShaderNodeVectorRefract.hpp>
#include <VisualShaderNodeVectorScalarMix.hpp>
#include <VisualShaderNodeVectorScalarSmoothStep.hpp>
#include <VisualShaderNodeVectorScalarStep.hpp>
#include <VisualShaderNodeVectorSmoothStep.hpp>
#include <WeakRef.hpp>
#include <WebRTCDataChannel.hpp>
#include <WebRTCDataChannelGDNative.hpp>
#include <WebRTCMultiplayer.hpp>
#include <WebRTCPeerConnection.hpp>
#include <WebRTCPeerConnectionGDNative.hpp>
#include <WebSocketClient.hpp>
#include <WebSocketMultiplayerPeer.hpp>
#include <WebSocketPeer.hpp>
#include <WebSocketServer.hpp>
#include <WindowDialog.hpp>
#include <World.hpp>
#include <World2D.hpp>
#include <WorldEnvironment.hpp>
#include <X509Certificate.hpp>
#include <XMLParser.hpp>
#include <YSort.hpp>
#include <ClassDB.hpp>
#include <Directory.hpp>
#include <Engine.hpp>
#include <File.hpp>
#include <Geometry.hpp>
#include <JSON.hpp>
#include <Marshalls.hpp>
#include <Mutex.hpp>
#include <OS.hpp>
#include <ResourceLoader.hpp>
#include <ResourceSaver.hpp>
#include <Semaphore.hpp>
#include <Thread.hpp>
#include <VisualScriptEditor.hpp>


namespace godot {
void ___init_method_bindings()
{
	GlobalConstants::___init_method_bindings();
	ARVRAnchor::___init_method_bindings();
	ARVRCamera::___init_method_bindings();
	ARVRController::___init_method_bindings();
	ARVRInterface::___init_method_bindings();
	ARVRInterfaceGDNative::___init_method_bindings();
	ARVROrigin::___init_method_bindings();
	ARVRPositionalTracker::___init_method_bindings();
	ARVRServer::___init_method_bindings();
	AStar::___init_method_bindings();
	AStar2D::___init_method_bindings();
	AcceptDialog::___init_method_bindings();
	AnimatedSprite::___init_method_bindings();
	AnimatedSprite3D::___init_method_bindings();
	AnimatedTexture::___init_method_bindings();
	Animation::___init_method_bindings();
	AnimationNode::___init_method_bindings();
	AnimationNodeAdd2::___init_method_bindings();
	AnimationNodeAdd3::___init_method_bindings();
	AnimationNodeAnimation::___init_method_bindings();
	AnimationNodeBlend2::___init_method_bindings();
	AnimationNodeBlend3::___init_method_bindings();
	AnimationNodeBlendSpace1D::___init_method_bindings();
	AnimationNodeBlendSpace2D::___init_method_bindings();
	AnimationNodeBlendTree::___init_method_bindings();
	AnimationNodeOneShot::___init_method_bindings();
	AnimationNodeOutput::___init_method_bindings();
	AnimationNodeStateMachine::___init_method_bindings();
	AnimationNodeStateMachinePlayback::___init_method_bindings();
	AnimationNodeStateMachineTransition::___init_method_bindings();
	AnimationNodeTimeScale::___init_method_bindings();
	AnimationNodeTimeSeek::___init_method_bindings();
	AnimationNodeTransition::___init_method_bindings();
	AnimationPlayer::___init_method_bindings();
	AnimationRootNode::___init_method_bindings();
	AnimationTrackEditPlugin::___init_method_bindings();
	AnimationTree::___init_method_bindings();
	AnimationTreePlayer::___init_method_bindings();
	Area::___init_method_bindings();
	Area2D::___init_method_bindings();
	ArrayMesh::___init_method_bindings();
	AtlasTexture::___init_method_bindings();
	AudioBusLayout::___init_method_bindings();
	AudioEffect::___init_method_bindings();
	AudioEffectAmplify::___init_method_bindings();
	AudioEffectBandLimitFilter::___init_method_bindings();
	AudioEffectBandPassFilter::___init_method_bindings();
	AudioEffectChorus::___init_method_bindings();
	AudioEffectCompressor::___init_method_bindings();
	AudioEffectDelay::___init_method_bindings();
	AudioEffectDistortion::___init_method_bindings();
	AudioEffectEQ::___init_method_bindings();
	AudioEffectEQ10::___init_method_bindings();
	AudioEffectEQ21::___init_method_bindings();
	AudioEffectEQ6::___init_method_bindings();
	AudioEffectFilter::___init_method_bindings();
	AudioEffectHighPassFilter::___init_method_bindings();
	AudioEffectHighShelfFilter::___init_method_bindings();
	AudioEffectInstance::___init_method_bindings();
	AudioEffectLimiter::___init_method_bindings();
	AudioEffectLowPassFilter::___init_method_bindings();
	AudioEffectLowShelfFilter::___init_method_bindings();
	AudioEffectNotchFilter::___init_method_bindings();
	AudioEffectPanner::___init_method_bindings();
	AudioEffectPhaser::___init_method_bindings();
	AudioEffectPitchShift::___init_method_bindings();
	AudioEffectRecord::___init_method_bindings();
	AudioEffectReverb::___init_method_bindings();
	AudioEffectSpectrumAnalyzer::___init_method_bindings();
	AudioEffectSpectrumAnalyzerInstance::___init_method_bindings();
	AudioEffectStereoEnhance::___init_method_bindings();
	AudioServer::___init_method_bindings();
	AudioStream::___init_method_bindings();
	AudioStreamGenerator::___init_method_bindings();
	AudioStreamGeneratorPlayback::___init_method_bindings();
	AudioStreamMicrophone::___init_method_bindings();
	AudioStreamOGGVorbis::___init_method_bindings();
	AudioStreamPlayback::___init_method_bindings();
	AudioStreamPlaybackResampled::___init_method_bindings();
	AudioStreamPlayer::___init_method_bindings();
	AudioStreamPlayer2D::___init_method_bindings();
	AudioStreamPlayer3D::___init_method_bindings();
	AudioStreamRandomPitch::___init_method_bindings();
	AudioStreamSample::___init_method_bindings();
	BackBufferCopy::___init_method_bindings();
	BakedLightmap::___init_method_bindings();
	BakedLightmapData::___init_method_bindings();
	BaseButton::___init_method_bindings();
	BitMap::___init_method_bindings();
	BitmapFont::___init_method_bindings();
	Bone2D::___init_method_bindings();
	BoneAttachment::___init_method_bindings();
	BoxContainer::___init_method_bindings();
	BoxShape::___init_method_bindings();
	BulletPhysicsDirectBodyState::___init_method_bindings();
	BulletPhysicsServer::___init_method_bindings();
	Button::___init_method_bindings();
	ButtonGroup::___init_method_bindings();
	CPUParticles::___init_method_bindings();
	CPUParticles2D::___init_method_bindings();
	CSGBox::___init_method_bindings();
	CSGCombiner::___init_method_bindings();
	CSGCylinder::___init_method_bindings();
	CSGMesh::___init_method_bindings();
	CSGPolygon::___init_method_bindings();
	CSGPrimitive::___init_method_bindings();
	CSGShape::___init_method_bindings();
	CSGSphere::___init_method_bindings();
	CSGTorus::___init_method_bindings();
	Camera::___init_method_bindings();
	Camera2D::___init_method_bindings();
	CameraFeed::___init_method_bindings();
	CameraServer::___init_method_bindings();
	CameraTexture::___init_method_bindings();
	CanvasItem::___init_method_bindings();
	CanvasItemMaterial::___init_method_bindings();
	CanvasLayer::___init_method_bindings();
	CanvasModulate::___init_method_bindings();
	CapsuleMesh::___init_method_bindings();
	CapsuleShape::___init_method_bindings();
	CapsuleShape2D::___init_method_bindings();
	CenterContainer::___init_method_bindings();
	CharFXTransform::___init_method_bindings();
	CheckBox::___init_method_bindings();
	CheckButton::___init_method_bindings();
	CircleShape2D::___init_method_bindings();
	ClippedCamera::___init_method_bindings();
	CollisionObject::___init_method_bindings();
	CollisionObject2D::___init_method_bindings();
	CollisionPolygon::___init_method_bindings();
	CollisionPolygon2D::___init_method_bindings();
	CollisionShape::___init_method_bindings();
	CollisionShape2D::___init_method_bindings();
	ColorPicker::___init_method_bindings();
	ColorPickerButton::___init_method_bindings();
	ColorRect::___init_method_bindings();
	ConcavePolygonShape::___init_method_bindings();
	ConcavePolygonShape2D::___init_method_bindings();
	ConeTwistJoint::___init_method_bindings();
	ConfigFile::___init_method_bindings();
	ConfirmationDialog::___init_method_bindings();
	Container::___init_method_bindings();
	Control::___init_method_bindings();
	ConvexPolygonShape::___init_method_bindings();
	ConvexPolygonShape2D::___init_method_bindings();
	Crypto::___init_method_bindings();
	CryptoKey::___init_method_bindings();
	CubeMap::___init_method_bindings();
	CubeMesh::___init_method_bindings();
	Curve::___init_method_bindings();
	Curve2D::___init_method_bindings();
	Curve3D::___init_method_bindings();
	CurveTexture::___init_method_bindings();
	CylinderMesh::___init_method_bindings();
	CylinderShape::___init_method_bindings();
	DampedSpringJoint2D::___init_method_bindings();
	DirectionalLight::___init_method_bindings();
	DynamicFont::___init_method_bindings();
	DynamicFontData::___init_method_bindings();
	EditorExportPlugin::___init_method_bindings();
	EditorFeatureProfile::___init_method_bindings();
	EditorFileDialog::___init_method_bindings();
	EditorFileSystem::___init_method_bindings();
	EditorFileSystemDirectory::___init_method_bindings();
	EditorImportPlugin::___init_method_bindings();
	EditorInspector::___init_method_bindings();
	EditorInspectorPlugin::___init_method_bindings();
	EditorInterface::___init_method_bindings();
	EditorNavigationMeshGenerator::___init_method_bindings();
	EditorPlugin::___init_method_bindings();
	EditorProperty::___init_method_bindings();
	EditorResourceConversionPlugin::___init_method_bindings();
	EditorResourcePreview::___init_method_bindings();
	EditorResourcePreviewGenerator::___init_method_bindings();
	EditorSceneImporter::___init_method_bindings();
	EditorSceneImporterAssimp::___init_method_bindings();
	EditorScenePostImport::___init_method_bindings();
	EditorScript::___init_method_bindings();
	EditorSelection::___init_method_bindings();
	EditorSettings::___init_method_bindings();
	EditorSpatialGizmo::___init_method_bindings();
	EditorSpatialGizmoPlugin::___init_method_bindings();
	EditorSpinSlider::___init_method_bindings();
	EditorVCSInterface::___init_method_bindings();
	EncodedObjectAsID::___init_method_bindings();
	Environment::___init_method_bindings();
	Expression::___init_method_bindings();
	FileDialog::___init_method_bindings();
	Font::___init_method_bindings();
	FuncRef::___init_method_bindings();
	GDNative::___init_method_bindings();
	GDNativeLibrary::___init_method_bindings();
	GDScript::___init_method_bindings();
	GDScriptFunctionState::___init_method_bindings();
	GIProbe::___init_method_bindings();
	GIProbeData::___init_method_bindings();
	Generic6DOFJoint::___init_method_bindings();
	GeometryInstance::___init_method_bindings();
	Gradient::___init_method_bindings();
	GradientTexture::___init_method_bindings();
	GraphEdit::___init_method_bindings();
	GraphNode::___init_method_bindings();
	GridContainer::___init_method_bindings();
	GridMap::___init_method_bindings();
	GrooveJoint2D::___init_method_bindings();
	HBoxContainer::___init_method_bindings();
	HScrollBar::___init_method_bindings();
	HSeparator::___init_method_bindings();
	HSlider::___init_method_bindings();
	HSplitContainer::___init_method_bindings();
	HTTPClient::___init_method_bindings();
	HTTPRequest::___init_method_bindings();
	HashingContext::___init_method_bindings();
	HeightMapShape::___init_method_bindings();
	HingeJoint::___init_method_bindings();
	IP::___init_method_bindings();
	IP_Unix::___init_method_bindings();
	Image::___init_method_bindings();
	ImageTexture::___init_method_bindings();
	ImmediateGeometry::___init_method_bindings();
	Input::___init_method_bindings();
	InputDefault::___init_method_bindings();
	InputEvent::___init_method_bindings();
	InputEventAction::___init_method_bindings();
	InputEventGesture::___init_method_bindings();
	InputEventJoypadButton::___init_method_bindings();
	InputEventJoypadMotion::___init_method_bindings();
	InputEventKey::___init_method_bindings();
	InputEventMIDI::___init_method_bindings();
	InputEventMagnifyGesture::___init_method_bindings();
	InputEventMouse::___init_method_bindings();
	InputEventMouseButton::___init_method_bindings();
	InputEventMouseMotion::___init_method_bindings();
	InputEventPanGesture::___init_method_bindings();
	InputEventScreenDrag::___init_method_bindings();
	InputEventScreenTouch::___init_method_bindings();
	InputEventWithModifiers::___init_method_bindings();
	InputMap::___init_method_bindings();
	InstancePlaceholder::___init_method_bindings();
	InterpolatedCamera::___init_method_bindings();
	ItemList::___init_method_bindings();
	JSONParseResult::___init_method_bindings();
	JSONRPC::___init_method_bindings();
	JavaClass::___init_method_bindings();
	JavaClassWrapper::___init_method_bindings();
	JavaScript::___init_method_bindings();
	Joint::___init_method_bindings();
	Joint2D::___init_method_bindings();
	KinematicBody::___init_method_bindings();
	KinematicBody2D::___init_method_bindings();
	KinematicCollision::___init_method_bindings();
	KinematicCollision2D::___init_method_bindings();
	Label::___init_method_bindings();
	LargeTexture::___init_method_bindings();
	Light::___init_method_bindings();
	Light2D::___init_method_bindings();
	LightOccluder2D::___init_method_bindings();
	Line2D::___init_method_bindings();
	LineEdit::___init_method_bindings();
	LineShape2D::___init_method_bindings();
	LinkButton::___init_method_bindings();
	Listener::___init_method_bindings();
	MainLoop::___init_method_bindings();
	MarginContainer::___init_method_bindings();
	Material::___init_method_bindings();
	MenuButton::___init_method_bindings();
	Mesh::___init_method_bindings();
	MeshDataTool::___init_method_bindings();
	MeshInstance::___init_method_bindings();
	MeshInstance2D::___init_method_bindings();
	MeshLibrary::___init_method_bindings();
	MeshTexture::___init_method_bindings();
	MobileVRInterface::___init_method_bindings();
	MultiMesh::___init_method_bindings();
	MultiMeshInstance::___init_method_bindings();
	MultiMeshInstance2D::___init_method_bindings();
	MultiplayerAPI::___init_method_bindings();
	MultiplayerPeerGDNative::___init_method_bindings();
	NativeScript::___init_method_bindings();
	Navigation::___init_method_bindings();
	Navigation2D::___init_method_bindings();
	NavigationMesh::___init_method_bindings();
	NavigationMeshInstance::___init_method_bindings();
	NavigationPolygon::___init_method_bindings();
	NavigationPolygonInstance::___init_method_bindings();
	NetworkedMultiplayerENet::___init_method_bindings();
	NetworkedMultiplayerPeer::___init_method_bindings();
	NinePatchRect::___init_method_bindings();
	Node::___init_method_bindings();
	Node2D::___init_method_bindings();
	NoiseTexture::___init_method_bindings();
	Object::___init_method_bindings();
	OccluderPolygon2D::___init_method_bindings();
	OmniLight::___init_method_bindings();
	OpenSimplexNoise::___init_method_bindings();
	OptionButton::___init_method_bindings();
	PCKPacker::___init_method_bindings();
	PHashTranslation::___init_method_bindings();
	PackedDataContainer::___init_method_bindings();
	PackedDataContainerRef::___init_method_bindings();
	PackedScene::___init_method_bindings();
	PacketPeer::___init_method_bindings();
	PacketPeerGDNative::___init_method_bindings();
	PacketPeerStream::___init_method_bindings();
	PacketPeerUDP::___init_method_bindings();
	Panel::___init_method_bindings();
	PanelContainer::___init_method_bindings();
	PanoramaSky::___init_method_bindings();
	ParallaxBackground::___init_method_bindings();
	ParallaxLayer::___init_method_bindings();
	Particles::___init_method_bindings();
	Particles2D::___init_method_bindings();
	ParticlesMaterial::___init_method_bindings();
	Path::___init_method_bindings();
	Path2D::___init_method_bindings();
	PathFollow::___init_method_bindings();
	PathFollow2D::___init_method_bindings();
	Performance::___init_method_bindings();
	PhysicalBone::___init_method_bindings();
	Physics2DDirectBodyState::___init_method_bindings();
	Physics2DDirectBodyStateSW::___init_method_bindings();
	Physics2DDirectSpaceState::___init_method_bindings();
	Physics2DServer::___init_method_bindings();
	Physics2DServerSW::___init_method_bindings();
	Physics2DShapeQueryParameters::___init_method_bindings();
	Physics2DShapeQueryResult::___init_method_bindings();
	Physics2DTestMotionResult::___init_method_bindings();
	PhysicsBody::___init_method_bindings();
	PhysicsBody2D::___init_method_bindings();
	PhysicsDirectBodyState::___init_method_bindings();
	PhysicsDirectSpaceState::___init_method_bindings();
	PhysicsMaterial::___init_method_bindings();
	PhysicsServer::___init_method_bindings();
	PhysicsShapeQueryParameters::___init_method_bindings();
	PhysicsShapeQueryResult::___init_method_bindings();
	PinJoint::___init_method_bindings();
	PinJoint2D::___init_method_bindings();
	PlaneMesh::___init_method_bindings();
	PlaneShape::___init_method_bindings();
	PluginScript::___init_method_bindings();
	PointMesh::___init_method_bindings();
	Polygon2D::___init_method_bindings();
	PolygonPathFinder::___init_method_bindings();
	Popup::___init_method_bindings();
	PopupDialog::___init_method_bindings();
	PopupMenu::___init_method_bindings();
	PopupPanel::___init_method_bindings();
	Position2D::___init_method_bindings();
	Position3D::___init_method_bindings();
	PrimitiveMesh::___init_method_bindings();
	PrismMesh::___init_method_bindings();
	ProceduralSky::___init_method_bindings();
	ProgressBar::___init_method_bindings();
	ProjectSettings::___init_method_bindings();
	ProximityGroup::___init_method_bindings();
	ProxyTexture::___init_method_bindings();
	QuadMesh::___init_method_bindings();
	RandomNumberGenerator::___init_method_bindings();
	Range::___init_method_bindings();
	RayCast::___init_method_bindings();
	RayCast2D::___init_method_bindings();
	RayShape::___init_method_bindings();
	RayShape2D::___init_method_bindings();
	RectangleShape2D::___init_method_bindings();
	Reference::___init_method_bindings();
	ReferenceRect::___init_method_bindings();
	ReflectionProbe::___init_method_bindings();
	RegEx::___init_method_bindings();
	RegExMatch::___init_method_bindings();
	RemoteTransform::___init_method_bindings();
	RemoteTransform2D::___init_method_bindings();
	Resource::___init_method_bindings();
	ResourceFormatLoader::___init_method_bindings();
	ResourceFormatLoaderCrypto::___init_method_bindings();
	ResourceFormatSaver::___init_method_bindings();
	ResourceFormatSaverCrypto::___init_method_bindings();
	ResourceImporter::___init_method_bindings();
	ResourceInteractiveLoader::___init_method_bindings();
	ResourcePreloader::___init_method_bindings();
	RichTextEffect::___init_method_bindings();
	RichTextLabel::___init_method_bindings();
	RigidBody::___init_method_bindings();
	RigidBody2D::___init_method_bindings();
	RootMotionView::___init_method_bindings();
	SceneState::___init_method_bindings();
	SceneTree::___init_method_bindings();
	SceneTreeTimer::___init_method_bindings();
	Script::___init_method_bindings();
	ScriptCreateDialog::___init_method_bindings();
	ScriptEditor::___init_method_bindings();
	ScrollBar::___init_method_bindings();
	ScrollContainer::___init_method_bindings();
	SegmentShape2D::___init_method_bindings();
	Separator::___init_method_bindings();
	Shader::___init_method_bindings();
	ShaderMaterial::___init_method_bindings();
	Shape::___init_method_bindings();
	Shape2D::___init_method_bindings();
	ShortCut::___init_method_bindings();
	Skeleton::___init_method_bindings();
	Skeleton2D::___init_method_bindings();
	SkeletonIK::___init_method_bindings();
	Skin::___init_method_bindings();
	SkinReference::___init_method_bindings();
	Sky::___init_method_bindings();
	Slider::___init_method_bindings();
	SliderJoint::___init_method_bindings();
	SoftBody::___init_method_bindings();
	Spatial::___init_method_bindings();
	SpatialGizmo::___init_method_bindings();
	SpatialMaterial::___init_method_bindings();
	SpatialVelocityTracker::___init_method_bindings();
	SphereMesh::___init_method_bindings();
	SphereShape::___init_method_bindings();
	SpinBox::___init_method_bindings();
	SplitContainer::___init_method_bindings();
	SpotLight::___init_method_bindings();
	SpringArm::___init_method_bindings();
	Sprite::___init_method_bindings();
	Sprite3D::___init_method_bindings();
	SpriteBase3D::___init_method_bindings();
	SpriteFrames::___init_method_bindings();
	StaticBody::___init_method_bindings();
	StaticBody2D::___init_method_bindings();
	StreamPeer::___init_method_bindings();
	StreamPeerBuffer::___init_method_bindings();
	StreamPeerGDNative::___init_method_bindings();
	StreamPeerSSL::___init_method_bindings();
	StreamPeerTCP::___init_method_bindings();
	StreamTexture::___init_method_bindings();
	StyleBox::___init_method_bindings();
	StyleBoxEmpty::___init_method_bindings();
	StyleBoxFlat::___init_method_bindings();
	StyleBoxLine::___init_method_bindings();
	StyleBoxTexture::___init_method_bindings();
	SurfaceTool::___init_method_bindings();
	TCP_Server::___init_method_bindings();
	TabContainer::___init_method_bindings();
	Tabs::___init_method_bindings();
	TextEdit::___init_method_bindings();
	TextFile::___init_method_bindings();
	Texture::___init_method_bindings();
	Texture3D::___init_method_bindings();
	TextureArray::___init_method_bindings();
	TextureButton::___init_method_bindings();
	TextureLayered::___init_method_bindings();
	TextureProgress::___init_method_bindings();
	TextureRect::___init_method_bindings();
	Theme::___init_method_bindings();
	TileMap::___init_method_bindings();
	TileSet::___init_method_bindings();
	Timer::___init_method_bindings();
	ToolButton::___init_method_bindings();
	TouchScreenButton::___init_method_bindings();
	Translation::___init_method_bindings();
	TranslationServer::___init_method_bindings();
	Tree::___init_method_bindings();
	TreeItem::___init_method_bindings();
	TriangleMesh::___init_method_bindings();
	Tween::___init_method_bindings();
	UPNP::___init_method_bindings();
	UPNPDevice::___init_method_bindings();
	UndoRedo::___init_method_bindings();
	VBoxContainer::___init_method_bindings();
	VScrollBar::___init_method_bindings();
	VSeparator::___init_method_bindings();
	VSlider::___init_method_bindings();
	VSplitContainer::___init_method_bindings();
	VehicleBody::___init_method_bindings();
	VehicleWheel::___init_method_bindings();
	VideoPlayer::___init_method_bindings();
	VideoStream::___init_method_bindings();
	VideoStreamGDNative::___init_method_bindings();
	VideoStreamTheora::___init_method_bindings();
	VideoStreamWebm::___init_method_bindings();
	Viewport::___init_method_bindings();
	ViewportContainer::___init_method_bindings();
	ViewportTexture::___init_method_bindings();
	VisibilityEnabler::___init_method_bindings();
	VisibilityEnabler2D::___init_method_bindings();
	VisibilityNotifier::___init_method_bindings();
	VisibilityNotifier2D::___init_method_bindings();
	VisualInstance::___init_method_bindings();
	VisualScript::___init_method_bindings();
	VisualScriptBasicTypeConstant::___init_method_bindings();
	VisualScriptBuiltinFunc::___init_method_bindings();
	VisualScriptClassConstant::___init_method_bindings();
	VisualScriptComment::___init_method_bindings();
	VisualScriptComposeArray::___init_method_bindings();
	VisualScriptCondition::___init_method_bindings();
	VisualScriptConstant::___init_method_bindings();
	VisualScriptConstructor::___init_method_bindings();
	VisualScriptCustomNode::___init_method_bindings();
	VisualScriptDeconstruct::___init_method_bindings();
	VisualScriptEmitSignal::___init_method_bindings();
	VisualScriptEngineSingleton::___init_method_bindings();
	VisualScriptExpression::___init_method_bindings();
	VisualScriptFunction::___init_method_bindings();
	VisualScriptFunctionCall::___init_method_bindings();
	VisualScriptFunctionState::___init_method_bindings();
	VisualScriptGlobalConstant::___init_method_bindings();
	VisualScriptIndexGet::___init_method_bindings();
	VisualScriptIndexSet::___init_method_bindings();
	VisualScriptInputAction::___init_method_bindings();
	VisualScriptIterator::___init_method_bindings();
	VisualScriptLists::___init_method_bindings();
	VisualScriptLocalVar::___init_method_bindings();
	VisualScriptLocalVarSet::___init_method_bindings();
	VisualScriptMathConstant::___init_method_bindings();
	VisualScriptNode::___init_method_bindings();
	VisualScriptOperator::___init_method_bindings();
	VisualScriptPreload::___init_method_bindings();
	VisualScriptPropertyGet::___init_method_bindings();
	VisualScriptPropertySet::___init_method_bindings();
	VisualScriptResourcePath::___init_method_bindings();
	VisualScriptReturn::___init_method_bindings();
	VisualScriptSceneNode::___init_method_bindings();
	VisualScriptSceneTree::___init_method_bindings();
	VisualScriptSelect::___init_method_bindings();
	VisualScriptSelf::___init_method_bindings();
	VisualScriptSequence::___init_method_bindings();
	VisualScriptSubCall::___init_method_bindings();
	VisualScriptSwitch::___init_method_bindings();
	VisualScriptTypeCast::___init_method_bindings();
	VisualScriptVariableGet::___init_method_bindings();
	VisualScriptVariableSet::___init_method_bindings();
	VisualScriptWhile::___init_method_bindings();
	VisualScriptYield::___init_method_bindings();
	VisualScriptYieldSignal::___init_method_bindings();
	VisualServer::___init_method_bindings();
	VisualShader::___init_method_bindings();
	VisualShaderNode::___init_method_bindings();
	VisualShaderNodeBooleanConstant::___init_method_bindings();
	VisualShaderNodeBooleanUniform::___init_method_bindings();
	VisualShaderNodeColorConstant::___init_method_bindings();
	VisualShaderNodeColorFunc::___init_method_bindings();
	VisualShaderNodeColorOp::___init_method_bindings();
	VisualShaderNodeColorUniform::___init_method_bindings();
	VisualShaderNodeCompare::___init_method_bindings();
	VisualShaderNodeCubeMap::___init_method_bindings();
	VisualShaderNodeCubeMapUniform::___init_method_bindings();
	VisualShaderNodeCustom::___init_method_bindings();
	VisualShaderNodeDeterminant::___init_method_bindings();
	VisualShaderNodeDotProduct::___init_method_bindings();
	VisualShaderNodeExpression::___init_method_bindings();
	VisualShaderNodeFaceForward::___init_method_bindings();
	VisualShaderNodeFresnel::___init_method_bindings();
	VisualShaderNodeGlobalExpression::___init_method_bindings();
	VisualShaderNodeGroupBase::___init_method_bindings();
	VisualShaderNodeIf::___init_method_bindings();
	VisualShaderNodeInput::___init_method_bindings();
	VisualShaderNodeIs::___init_method_bindings();
	VisualShaderNodeOuterProduct::___init_method_bindings();
	VisualShaderNodeOutput::___init_method_bindings();
	VisualShaderNodeScalarClamp::___init_method_bindings();
	VisualShaderNodeScalarConstant::___init_method_bindings();
	VisualShaderNodeScalarDerivativeFunc::___init_method_bindings();
	VisualShaderNodeScalarFunc::___init_method_bindings();
	VisualShaderNodeScalarInterp::___init_method_bindings();
	VisualShaderNodeScalarOp::___init_method_bindings();
	VisualShaderNodeScalarSmoothStep::___init_method_bindings();
	VisualShaderNodeScalarSwitch::___init_method_bindings();
	VisualShaderNodeScalarUniform::___init_method_bindings();
	VisualShaderNodeSwitch::___init_method_bindings();
	VisualShaderNodeTexture::___init_method_bindings();
	VisualShaderNodeTextureUniform::___init_method_bindings();
	VisualShaderNodeTextureUniformTriplanar::___init_method_bindings();
	VisualShaderNodeTransformCompose::___init_method_bindings();
	VisualShaderNodeTransformConstant::___init_method_bindings();
	VisualShaderNodeTransformDecompose::___init_method_bindings();
	VisualShaderNodeTransformFunc::___init_method_bindings();
	VisualShaderNodeTransformMult::___init_method_bindings();
	VisualShaderNodeTransformUniform::___init_method_bindings();
	VisualShaderNodeTransformVecMult::___init_method_bindings();
	VisualShaderNodeUniform::___init_method_bindings();
	VisualShaderNodeVec3Constant::___init_method_bindings();
	VisualShaderNodeVec3Uniform::___init_method_bindings();
	VisualShaderNodeVectorClamp::___init_method_bindings();
	VisualShaderNodeVectorCompose::___init_method_bindings();
	VisualShaderNodeVectorDecompose::___init_method_bindings();
	VisualShaderNodeVectorDerivativeFunc::___init_method_bindings();
	VisualShaderNodeVectorDistance::___init_method_bindings();
	VisualShaderNodeVectorFunc::___init_method_bindings();
	VisualShaderNodeVectorInterp::___init_method_bindings();
	VisualShaderNodeVectorLen::___init_method_bindings();
	VisualShaderNodeVectorOp::___init_method_bindings();
	VisualShaderNodeVectorRefract::___init_method_bindings();
	VisualShaderNodeVectorScalarMix::___init_method_bindings();
	VisualShaderNodeVectorScalarSmoothStep::___init_method_bindings();
	VisualShaderNodeVectorScalarStep::___init_method_bindings();
	VisualShaderNodeVectorSmoothStep::___init_method_bindings();
	WeakRef::___init_method_bindings();
	WebRTCDataChannel::___init_method_bindings();
	WebRTCDataChannelGDNative::___init_method_bindings();
	WebRTCMultiplayer::___init_method_bindings();
	WebRTCPeerConnection::___init_method_bindings();
	WebRTCPeerConnectionGDNative::___init_method_bindings();
	WebSocketClient::___init_method_bindings();
	WebSocketMultiplayerPeer::___init_method_bindings();
	WebSocketPeer::___init_method_bindings();
	WebSocketServer::___init_method_bindings();
	WindowDialog::___init_method_bindings();
	World::___init_method_bindings();
	World2D::___init_method_bindings();
	WorldEnvironment::___init_method_bindings();
	X509Certificate::___init_method_bindings();
	XMLParser::___init_method_bindings();
	YSort::___init_method_bindings();
	ClassDB::___init_method_bindings();
	Directory::___init_method_bindings();
	Engine::___init_method_bindings();
	File::___init_method_bindings();
	Geometry::___init_method_bindings();
	JSON::___init_method_bindings();
	Marshalls::___init_method_bindings();
	Mutex::___init_method_bindings();
	OS::___init_method_bindings();
	ResourceLoader::___init_method_bindings();
	ResourceSaver::___init_method_bindings();
	Semaphore::___init_method_bindings();
	Thread::___init_method_bindings();
	VisualScriptEditor::___init_method_bindings();
}

}