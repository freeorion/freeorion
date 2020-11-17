#include "TagDB.hpp"
#include <typeinfo>


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
void ___register_types()
{
	godot::_TagDB::register_global_type("GlobalConstants", typeid(GlobalConstants).hash_code(), 0);
	godot::_TagDB::register_global_type("ARVRAnchor", typeid(ARVRAnchor).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("ARVRCamera", typeid(ARVRCamera).hash_code(), typeid(Camera).hash_code());
	godot::_TagDB::register_global_type("ARVRController", typeid(ARVRController).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("ARVRInterface", typeid(ARVRInterface).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("ARVRInterfaceGDNative", typeid(ARVRInterfaceGDNative).hash_code(), typeid(ARVRInterface).hash_code());
	godot::_TagDB::register_global_type("ARVROrigin", typeid(ARVROrigin).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("ARVRPositionalTracker", typeid(ARVRPositionalTracker).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("ARVRServer", typeid(ARVRServer).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("AStar", typeid(AStar).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("AStar2D", typeid(AStar2D).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("AcceptDialog", typeid(AcceptDialog).hash_code(), typeid(WindowDialog).hash_code());
	godot::_TagDB::register_global_type("AnimatedSprite", typeid(AnimatedSprite).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("AnimatedSprite3D", typeid(AnimatedSprite3D).hash_code(), typeid(SpriteBase3D).hash_code());
	godot::_TagDB::register_global_type("AnimatedTexture", typeid(AnimatedTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("Animation", typeid(Animation).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("AnimationNode", typeid(AnimationNode).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeAdd2", typeid(AnimationNodeAdd2).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeAdd3", typeid(AnimationNodeAdd3).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeAnimation", typeid(AnimationNodeAnimation).hash_code(), typeid(AnimationRootNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeBlend2", typeid(AnimationNodeBlend2).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeBlend3", typeid(AnimationNodeBlend3).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeBlendSpace1D", typeid(AnimationNodeBlendSpace1D).hash_code(), typeid(AnimationRootNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeBlendSpace2D", typeid(AnimationNodeBlendSpace2D).hash_code(), typeid(AnimationRootNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeBlendTree", typeid(AnimationNodeBlendTree).hash_code(), typeid(AnimationRootNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeOneShot", typeid(AnimationNodeOneShot).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeOutput", typeid(AnimationNodeOutput).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeStateMachine", typeid(AnimationNodeStateMachine).hash_code(), typeid(AnimationRootNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeStateMachinePlayback", typeid(AnimationNodeStateMachinePlayback).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeStateMachineTransition", typeid(AnimationNodeStateMachineTransition).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeTimeScale", typeid(AnimationNodeTimeScale).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeTimeSeek", typeid(AnimationNodeTimeSeek).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationNodeTransition", typeid(AnimationNodeTransition).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationPlayer", typeid(AnimationPlayer).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("AnimationRootNode", typeid(AnimationRootNode).hash_code(), typeid(AnimationNode).hash_code());
	godot::_TagDB::register_global_type("AnimationTrackEditPlugin", typeid(AnimationTrackEditPlugin).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("AnimationTree", typeid(AnimationTree).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("AnimationTreePlayer", typeid(AnimationTreePlayer).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("Area", typeid(Area).hash_code(), typeid(CollisionObject).hash_code());
	godot::_TagDB::register_global_type("Area2D", typeid(Area2D).hash_code(), typeid(CollisionObject2D).hash_code());
	godot::_TagDB::register_global_type("ArrayMesh", typeid(ArrayMesh).hash_code(), typeid(Mesh).hash_code());
	godot::_TagDB::register_global_type("AtlasTexture", typeid(AtlasTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("AudioBusLayout", typeid(AudioBusLayout).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("AudioEffect", typeid(AudioEffect).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("AudioEffectAmplify", typeid(AudioEffectAmplify).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectBandLimitFilter", typeid(AudioEffectBandLimitFilter).hash_code(), typeid(AudioEffectFilter).hash_code());
	godot::_TagDB::register_global_type("AudioEffectBandPassFilter", typeid(AudioEffectBandPassFilter).hash_code(), typeid(AudioEffectFilter).hash_code());
	godot::_TagDB::register_global_type("AudioEffectChorus", typeid(AudioEffectChorus).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectCompressor", typeid(AudioEffectCompressor).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectDelay", typeid(AudioEffectDelay).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectDistortion", typeid(AudioEffectDistortion).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectEQ", typeid(AudioEffectEQ).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectEQ10", typeid(AudioEffectEQ10).hash_code(), typeid(AudioEffectEQ).hash_code());
	godot::_TagDB::register_global_type("AudioEffectEQ21", typeid(AudioEffectEQ21).hash_code(), typeid(AudioEffectEQ).hash_code());
	godot::_TagDB::register_global_type("AudioEffectEQ6", typeid(AudioEffectEQ6).hash_code(), typeid(AudioEffectEQ).hash_code());
	godot::_TagDB::register_global_type("AudioEffectFilter", typeid(AudioEffectFilter).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectHighPassFilter", typeid(AudioEffectHighPassFilter).hash_code(), typeid(AudioEffectFilter).hash_code());
	godot::_TagDB::register_global_type("AudioEffectHighShelfFilter", typeid(AudioEffectHighShelfFilter).hash_code(), typeid(AudioEffectFilter).hash_code());
	godot::_TagDB::register_global_type("AudioEffectInstance", typeid(AudioEffectInstance).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("AudioEffectLimiter", typeid(AudioEffectLimiter).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectLowPassFilter", typeid(AudioEffectLowPassFilter).hash_code(), typeid(AudioEffectFilter).hash_code());
	godot::_TagDB::register_global_type("AudioEffectLowShelfFilter", typeid(AudioEffectLowShelfFilter).hash_code(), typeid(AudioEffectFilter).hash_code());
	godot::_TagDB::register_global_type("AudioEffectNotchFilter", typeid(AudioEffectNotchFilter).hash_code(), typeid(AudioEffectFilter).hash_code());
	godot::_TagDB::register_global_type("AudioEffectPanner", typeid(AudioEffectPanner).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectPhaser", typeid(AudioEffectPhaser).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectPitchShift", typeid(AudioEffectPitchShift).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectRecord", typeid(AudioEffectRecord).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectReverb", typeid(AudioEffectReverb).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectSpectrumAnalyzer", typeid(AudioEffectSpectrumAnalyzer).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioEffectSpectrumAnalyzerInstance", typeid(AudioEffectSpectrumAnalyzerInstance).hash_code(), typeid(AudioEffectInstance).hash_code());
	godot::_TagDB::register_global_type("AudioEffectStereoEnhance", typeid(AudioEffectStereoEnhance).hash_code(), typeid(AudioEffect).hash_code());
	godot::_TagDB::register_global_type("AudioServer", typeid(AudioServer).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("AudioStream", typeid(AudioStream).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("AudioStreamGenerator", typeid(AudioStreamGenerator).hash_code(), typeid(AudioStream).hash_code());
	godot::_TagDB::register_global_type("AudioStreamGeneratorPlayback", typeid(AudioStreamGeneratorPlayback).hash_code(), typeid(AudioStreamPlaybackResampled).hash_code());
	godot::_TagDB::register_global_type("AudioStreamMicrophone", typeid(AudioStreamMicrophone).hash_code(), typeid(AudioStream).hash_code());
	godot::_TagDB::register_global_type("AudioStreamOGGVorbis", typeid(AudioStreamOGGVorbis).hash_code(), typeid(AudioStream).hash_code());
	godot::_TagDB::register_global_type("AudioStreamPlayback", typeid(AudioStreamPlayback).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("AudioStreamPlaybackResampled", typeid(AudioStreamPlaybackResampled).hash_code(), typeid(AudioStreamPlayback).hash_code());
	godot::_TagDB::register_global_type("AudioStreamPlayer", typeid(AudioStreamPlayer).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("AudioStreamPlayer2D", typeid(AudioStreamPlayer2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("AudioStreamPlayer3D", typeid(AudioStreamPlayer3D).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("AudioStreamRandomPitch", typeid(AudioStreamRandomPitch).hash_code(), typeid(AudioStream).hash_code());
	godot::_TagDB::register_global_type("AudioStreamSample", typeid(AudioStreamSample).hash_code(), typeid(AudioStream).hash_code());
	godot::_TagDB::register_global_type("BackBufferCopy", typeid(BackBufferCopy).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("BakedLightmap", typeid(BakedLightmap).hash_code(), typeid(VisualInstance).hash_code());
	godot::_TagDB::register_global_type("BakedLightmapData", typeid(BakedLightmapData).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("BaseButton", typeid(BaseButton).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("BitMap", typeid(BitMap).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("BitmapFont", typeid(BitmapFont).hash_code(), typeid(Font).hash_code());
	godot::_TagDB::register_global_type("Bone2D", typeid(Bone2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("BoneAttachment", typeid(BoneAttachment).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("BoxContainer", typeid(BoxContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("BoxShape", typeid(BoxShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("BulletPhysicsDirectBodyState", typeid(BulletPhysicsDirectBodyState).hash_code(), typeid(PhysicsDirectBodyState).hash_code());
	godot::_TagDB::register_global_type("BulletPhysicsServer", typeid(BulletPhysicsServer).hash_code(), typeid(PhysicsServer).hash_code());
	godot::_TagDB::register_global_type("Button", typeid(Button).hash_code(), typeid(BaseButton).hash_code());
	godot::_TagDB::register_global_type("ButtonGroup", typeid(ButtonGroup).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("CPUParticles", typeid(CPUParticles).hash_code(), typeid(GeometryInstance).hash_code());
	godot::_TagDB::register_global_type("CPUParticles2D", typeid(CPUParticles2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("CSGBox", typeid(CSGBox).hash_code(), typeid(CSGPrimitive).hash_code());
	godot::_TagDB::register_global_type("CSGCombiner", typeid(CSGCombiner).hash_code(), typeid(CSGShape).hash_code());
	godot::_TagDB::register_global_type("CSGCylinder", typeid(CSGCylinder).hash_code(), typeid(CSGPrimitive).hash_code());
	godot::_TagDB::register_global_type("CSGMesh", typeid(CSGMesh).hash_code(), typeid(CSGPrimitive).hash_code());
	godot::_TagDB::register_global_type("CSGPolygon", typeid(CSGPolygon).hash_code(), typeid(CSGPrimitive).hash_code());
	godot::_TagDB::register_global_type("CSGPrimitive", typeid(CSGPrimitive).hash_code(), typeid(CSGShape).hash_code());
	godot::_TagDB::register_global_type("CSGShape", typeid(CSGShape).hash_code(), typeid(GeometryInstance).hash_code());
	godot::_TagDB::register_global_type("CSGSphere", typeid(CSGSphere).hash_code(), typeid(CSGPrimitive).hash_code());
	godot::_TagDB::register_global_type("CSGTorus", typeid(CSGTorus).hash_code(), typeid(CSGPrimitive).hash_code());
	godot::_TagDB::register_global_type("Camera", typeid(Camera).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("Camera2D", typeid(Camera2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("CameraFeed", typeid(CameraFeed).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("CameraServer", typeid(CameraServer).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("CameraTexture", typeid(CameraTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("CanvasItem", typeid(CanvasItem).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("CanvasItemMaterial", typeid(CanvasItemMaterial).hash_code(), typeid(Material).hash_code());
	godot::_TagDB::register_global_type("CanvasLayer", typeid(CanvasLayer).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("CanvasModulate", typeid(CanvasModulate).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("CapsuleMesh", typeid(CapsuleMesh).hash_code(), typeid(PrimitiveMesh).hash_code());
	godot::_TagDB::register_global_type("CapsuleShape", typeid(CapsuleShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("CapsuleShape2D", typeid(CapsuleShape2D).hash_code(), typeid(Shape2D).hash_code());
	godot::_TagDB::register_global_type("CenterContainer", typeid(CenterContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("CharFXTransform", typeid(CharFXTransform).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("CheckBox", typeid(CheckBox).hash_code(), typeid(Button).hash_code());
	godot::_TagDB::register_global_type("CheckButton", typeid(CheckButton).hash_code(), typeid(Button).hash_code());
	godot::_TagDB::register_global_type("CircleShape2D", typeid(CircleShape2D).hash_code(), typeid(Shape2D).hash_code());
	godot::_TagDB::register_global_type("ClippedCamera", typeid(ClippedCamera).hash_code(), typeid(Camera).hash_code());
	godot::_TagDB::register_global_type("CollisionObject", typeid(CollisionObject).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("CollisionObject2D", typeid(CollisionObject2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("CollisionPolygon", typeid(CollisionPolygon).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("CollisionPolygon2D", typeid(CollisionPolygon2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("CollisionShape", typeid(CollisionShape).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("CollisionShape2D", typeid(CollisionShape2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("ColorPicker", typeid(ColorPicker).hash_code(), typeid(BoxContainer).hash_code());
	godot::_TagDB::register_global_type("ColorPickerButton", typeid(ColorPickerButton).hash_code(), typeid(Button).hash_code());
	godot::_TagDB::register_global_type("ColorRect", typeid(ColorRect).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("ConcavePolygonShape", typeid(ConcavePolygonShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("ConcavePolygonShape2D", typeid(ConcavePolygonShape2D).hash_code(), typeid(Shape2D).hash_code());
	godot::_TagDB::register_global_type("ConeTwistJoint", typeid(ConeTwistJoint).hash_code(), typeid(Joint).hash_code());
	godot::_TagDB::register_global_type("ConfigFile", typeid(ConfigFile).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("ConfirmationDialog", typeid(ConfirmationDialog).hash_code(), typeid(AcceptDialog).hash_code());
	godot::_TagDB::register_global_type("Container", typeid(Container).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("Control", typeid(Control).hash_code(), typeid(CanvasItem).hash_code());
	godot::_TagDB::register_global_type("ConvexPolygonShape", typeid(ConvexPolygonShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("ConvexPolygonShape2D", typeid(ConvexPolygonShape2D).hash_code(), typeid(Shape2D).hash_code());
	godot::_TagDB::register_global_type("Crypto", typeid(Crypto).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("CryptoKey", typeid(CryptoKey).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("CubeMap", typeid(CubeMap).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("CubeMesh", typeid(CubeMesh).hash_code(), typeid(PrimitiveMesh).hash_code());
	godot::_TagDB::register_global_type("Curve", typeid(Curve).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Curve2D", typeid(Curve2D).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Curve3D", typeid(Curve3D).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("CurveTexture", typeid(CurveTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("CylinderMesh", typeid(CylinderMesh).hash_code(), typeid(PrimitiveMesh).hash_code());
	godot::_TagDB::register_global_type("CylinderShape", typeid(CylinderShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("DampedSpringJoint2D", typeid(DampedSpringJoint2D).hash_code(), typeid(Joint2D).hash_code());
	godot::_TagDB::register_global_type("DirectionalLight", typeid(DirectionalLight).hash_code(), typeid(Light).hash_code());
	godot::_TagDB::register_global_type("DynamicFont", typeid(DynamicFont).hash_code(), typeid(Font).hash_code());
	godot::_TagDB::register_global_type("DynamicFontData", typeid(DynamicFontData).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("EditorExportPlugin", typeid(EditorExportPlugin).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("EditorFeatureProfile", typeid(EditorFeatureProfile).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("EditorFileDialog", typeid(EditorFileDialog).hash_code(), typeid(ConfirmationDialog).hash_code());
	godot::_TagDB::register_global_type("EditorFileSystem", typeid(EditorFileSystem).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("EditorFileSystemDirectory", typeid(EditorFileSystemDirectory).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("EditorImportPlugin", typeid(EditorImportPlugin).hash_code(), typeid(ResourceImporter).hash_code());
	godot::_TagDB::register_global_type("EditorInspector", typeid(EditorInspector).hash_code(), typeid(ScrollContainer).hash_code());
	godot::_TagDB::register_global_type("EditorInspectorPlugin", typeid(EditorInspectorPlugin).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("EditorInterface", typeid(EditorInterface).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("EditorNavigationMeshGenerator", typeid(EditorNavigationMeshGenerator).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("EditorPlugin", typeid(EditorPlugin).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("EditorProperty", typeid(EditorProperty).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("EditorResourceConversionPlugin", typeid(EditorResourceConversionPlugin).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("EditorResourcePreview", typeid(EditorResourcePreview).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("EditorResourcePreviewGenerator", typeid(EditorResourcePreviewGenerator).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("EditorSceneImporter", typeid(EditorSceneImporter).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("EditorSceneImporterAssimp", typeid(EditorSceneImporterAssimp).hash_code(), typeid(EditorSceneImporter).hash_code());
	godot::_TagDB::register_global_type("EditorScenePostImport", typeid(EditorScenePostImport).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("EditorScript", typeid(EditorScript).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("EditorSelection", typeid(EditorSelection).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("EditorSettings", typeid(EditorSettings).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("EditorSpatialGizmo", typeid(EditorSpatialGizmo).hash_code(), typeid(SpatialGizmo).hash_code());
	godot::_TagDB::register_global_type("EditorSpatialGizmoPlugin", typeid(EditorSpatialGizmoPlugin).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("EditorSpinSlider", typeid(EditorSpinSlider).hash_code(), typeid(Range).hash_code());
	godot::_TagDB::register_global_type("EditorVCSInterface", typeid(EditorVCSInterface).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("EncodedObjectAsID", typeid(EncodedObjectAsID).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("Environment", typeid(Environment).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Expression", typeid(Expression).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("FileDialog", typeid(FileDialog).hash_code(), typeid(ConfirmationDialog).hash_code());
	godot::_TagDB::register_global_type("Font", typeid(Font).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("FuncRef", typeid(FuncRef).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("GDNative", typeid(GDNative).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("GDNativeLibrary", typeid(GDNativeLibrary).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("GDScript", typeid(GDScript).hash_code(), typeid(Script).hash_code());
	godot::_TagDB::register_global_type("GDScriptFunctionState", typeid(GDScriptFunctionState).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("GIProbe", typeid(GIProbe).hash_code(), typeid(VisualInstance).hash_code());
	godot::_TagDB::register_global_type("GIProbeData", typeid(GIProbeData).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Generic6DOFJoint", typeid(Generic6DOFJoint).hash_code(), typeid(Joint).hash_code());
	godot::_TagDB::register_global_type("GeometryInstance", typeid(GeometryInstance).hash_code(), typeid(VisualInstance).hash_code());
	godot::_TagDB::register_global_type("Gradient", typeid(Gradient).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("GradientTexture", typeid(GradientTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("GraphEdit", typeid(GraphEdit).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("GraphNode", typeid(GraphNode).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("GridContainer", typeid(GridContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("GridMap", typeid(GridMap).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("GrooveJoint2D", typeid(GrooveJoint2D).hash_code(), typeid(Joint2D).hash_code());
	godot::_TagDB::register_global_type("HBoxContainer", typeid(HBoxContainer).hash_code(), typeid(BoxContainer).hash_code());
	godot::_TagDB::register_global_type("HScrollBar", typeid(HScrollBar).hash_code(), typeid(ScrollBar).hash_code());
	godot::_TagDB::register_global_type("HSeparator", typeid(HSeparator).hash_code(), typeid(Separator).hash_code());
	godot::_TagDB::register_global_type("HSlider", typeid(HSlider).hash_code(), typeid(Slider).hash_code());
	godot::_TagDB::register_global_type("HSplitContainer", typeid(HSplitContainer).hash_code(), typeid(SplitContainer).hash_code());
	godot::_TagDB::register_global_type("HTTPClient", typeid(HTTPClient).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("HTTPRequest", typeid(HTTPRequest).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("HashingContext", typeid(HashingContext).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("HeightMapShape", typeid(HeightMapShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("HingeJoint", typeid(HingeJoint).hash_code(), typeid(Joint).hash_code());
	godot::_TagDB::register_global_type("IP", typeid(IP).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("IP_Unix", typeid(IP_Unix).hash_code(), typeid(IP).hash_code());
	godot::_TagDB::register_global_type("Image", typeid(Image).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("ImageTexture", typeid(ImageTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("ImmediateGeometry", typeid(ImmediateGeometry).hash_code(), typeid(GeometryInstance).hash_code());
	godot::_TagDB::register_global_type("Input", typeid(Input).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("InputDefault", typeid(InputDefault).hash_code(), typeid(Input).hash_code());
	godot::_TagDB::register_global_type("InputEvent", typeid(InputEvent).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("InputEventAction", typeid(InputEventAction).hash_code(), typeid(InputEvent).hash_code());
	godot::_TagDB::register_global_type("InputEventGesture", typeid(InputEventGesture).hash_code(), typeid(InputEventWithModifiers).hash_code());
	godot::_TagDB::register_global_type("InputEventJoypadButton", typeid(InputEventJoypadButton).hash_code(), typeid(InputEvent).hash_code());
	godot::_TagDB::register_global_type("InputEventJoypadMotion", typeid(InputEventJoypadMotion).hash_code(), typeid(InputEvent).hash_code());
	godot::_TagDB::register_global_type("InputEventKey", typeid(InputEventKey).hash_code(), typeid(InputEventWithModifiers).hash_code());
	godot::_TagDB::register_global_type("InputEventMIDI", typeid(InputEventMIDI).hash_code(), typeid(InputEvent).hash_code());
	godot::_TagDB::register_global_type("InputEventMagnifyGesture", typeid(InputEventMagnifyGesture).hash_code(), typeid(InputEventGesture).hash_code());
	godot::_TagDB::register_global_type("InputEventMouse", typeid(InputEventMouse).hash_code(), typeid(InputEventWithModifiers).hash_code());
	godot::_TagDB::register_global_type("InputEventMouseButton", typeid(InputEventMouseButton).hash_code(), typeid(InputEventMouse).hash_code());
	godot::_TagDB::register_global_type("InputEventMouseMotion", typeid(InputEventMouseMotion).hash_code(), typeid(InputEventMouse).hash_code());
	godot::_TagDB::register_global_type("InputEventPanGesture", typeid(InputEventPanGesture).hash_code(), typeid(InputEventGesture).hash_code());
	godot::_TagDB::register_global_type("InputEventScreenDrag", typeid(InputEventScreenDrag).hash_code(), typeid(InputEvent).hash_code());
	godot::_TagDB::register_global_type("InputEventScreenTouch", typeid(InputEventScreenTouch).hash_code(), typeid(InputEvent).hash_code());
	godot::_TagDB::register_global_type("InputEventWithModifiers", typeid(InputEventWithModifiers).hash_code(), typeid(InputEvent).hash_code());
	godot::_TagDB::register_global_type("InputMap", typeid(InputMap).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("InstancePlaceholder", typeid(InstancePlaceholder).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("InterpolatedCamera", typeid(InterpolatedCamera).hash_code(), typeid(Camera).hash_code());
	godot::_TagDB::register_global_type("ItemList", typeid(ItemList).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("JSONParseResult", typeid(JSONParseResult).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("JSONRPC", typeid(JSONRPC).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("JavaClass", typeid(JavaClass).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("JavaClassWrapper", typeid(JavaClassWrapper).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("JavaScript", typeid(JavaScript).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("Joint", typeid(Joint).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("Joint2D", typeid(Joint2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("KinematicBody", typeid(KinematicBody).hash_code(), typeid(PhysicsBody).hash_code());
	godot::_TagDB::register_global_type("KinematicBody2D", typeid(KinematicBody2D).hash_code(), typeid(PhysicsBody2D).hash_code());
	godot::_TagDB::register_global_type("KinematicCollision", typeid(KinematicCollision).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("KinematicCollision2D", typeid(KinematicCollision2D).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("Label", typeid(Label).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("LargeTexture", typeid(LargeTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("Light", typeid(Light).hash_code(), typeid(VisualInstance).hash_code());
	godot::_TagDB::register_global_type("Light2D", typeid(Light2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("LightOccluder2D", typeid(LightOccluder2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("Line2D", typeid(Line2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("LineEdit", typeid(LineEdit).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("LineShape2D", typeid(LineShape2D).hash_code(), typeid(Shape2D).hash_code());
	godot::_TagDB::register_global_type("LinkButton", typeid(LinkButton).hash_code(), typeid(BaseButton).hash_code());
	godot::_TagDB::register_global_type("Listener", typeid(Listener).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("MainLoop", typeid(MainLoop).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("MarginContainer", typeid(MarginContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("Material", typeid(Material).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("MenuButton", typeid(MenuButton).hash_code(), typeid(Button).hash_code());
	godot::_TagDB::register_global_type("Mesh", typeid(Mesh).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("MeshDataTool", typeid(MeshDataTool).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("MeshInstance", typeid(MeshInstance).hash_code(), typeid(GeometryInstance).hash_code());
	godot::_TagDB::register_global_type("MeshInstance2D", typeid(MeshInstance2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("MeshLibrary", typeid(MeshLibrary).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("MeshTexture", typeid(MeshTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("MobileVRInterface", typeid(MobileVRInterface).hash_code(), typeid(ARVRInterface).hash_code());
	godot::_TagDB::register_global_type("MultiMesh", typeid(MultiMesh).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("MultiMeshInstance", typeid(MultiMeshInstance).hash_code(), typeid(GeometryInstance).hash_code());
	godot::_TagDB::register_global_type("MultiMeshInstance2D", typeid(MultiMeshInstance2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("MultiplayerAPI", typeid(MultiplayerAPI).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("MultiplayerPeerGDNative", typeid(MultiplayerPeerGDNative).hash_code(), typeid(NetworkedMultiplayerPeer).hash_code());
	godot::_TagDB::register_global_type("NativeScript", typeid(NativeScript).hash_code(), typeid(Script).hash_code());
	godot::_TagDB::register_global_type("Navigation", typeid(Navigation).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("Navigation2D", typeid(Navigation2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("NavigationMesh", typeid(NavigationMesh).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("NavigationMeshInstance", typeid(NavigationMeshInstance).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("NavigationPolygon", typeid(NavigationPolygon).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("NavigationPolygonInstance", typeid(NavigationPolygonInstance).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("NetworkedMultiplayerENet", typeid(NetworkedMultiplayerENet).hash_code(), typeid(NetworkedMultiplayerPeer).hash_code());
	godot::_TagDB::register_global_type("NetworkedMultiplayerPeer", typeid(NetworkedMultiplayerPeer).hash_code(), typeid(PacketPeer).hash_code());
	godot::_TagDB::register_global_type("NinePatchRect", typeid(NinePatchRect).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("Node", typeid(Node).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("Node2D", typeid(Node2D).hash_code(), typeid(CanvasItem).hash_code());
	godot::_TagDB::register_global_type("NoiseTexture", typeid(NoiseTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("Object", typeid(Object).hash_code(), 0);
	godot::_TagDB::register_global_type("OccluderPolygon2D", typeid(OccluderPolygon2D).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("OmniLight", typeid(OmniLight).hash_code(), typeid(Light).hash_code());
	godot::_TagDB::register_global_type("OpenSimplexNoise", typeid(OpenSimplexNoise).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("OptionButton", typeid(OptionButton).hash_code(), typeid(Button).hash_code());
	godot::_TagDB::register_global_type("PCKPacker", typeid(PCKPacker).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("PHashTranslation", typeid(PHashTranslation).hash_code(), typeid(Translation).hash_code());
	godot::_TagDB::register_global_type("PackedDataContainer", typeid(PackedDataContainer).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("PackedDataContainerRef", typeid(PackedDataContainerRef).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("PackedScene", typeid(PackedScene).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("PacketPeer", typeid(PacketPeer).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("PacketPeerGDNative", typeid(PacketPeerGDNative).hash_code(), typeid(PacketPeer).hash_code());
	godot::_TagDB::register_global_type("PacketPeerStream", typeid(PacketPeerStream).hash_code(), typeid(PacketPeer).hash_code());
	godot::_TagDB::register_global_type("PacketPeerUDP", typeid(PacketPeerUDP).hash_code(), typeid(PacketPeer).hash_code());
	godot::_TagDB::register_global_type("Panel", typeid(Panel).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("PanelContainer", typeid(PanelContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("PanoramaSky", typeid(PanoramaSky).hash_code(), typeid(Sky).hash_code());
	godot::_TagDB::register_global_type("ParallaxBackground", typeid(ParallaxBackground).hash_code(), typeid(CanvasLayer).hash_code());
	godot::_TagDB::register_global_type("ParallaxLayer", typeid(ParallaxLayer).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("Particles", typeid(Particles).hash_code(), typeid(GeometryInstance).hash_code());
	godot::_TagDB::register_global_type("Particles2D", typeid(Particles2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("ParticlesMaterial", typeid(ParticlesMaterial).hash_code(), typeid(Material).hash_code());
	godot::_TagDB::register_global_type("Path", typeid(Path).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("Path2D", typeid(Path2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("PathFollow", typeid(PathFollow).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("PathFollow2D", typeid(PathFollow2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("Performance", typeid(Performance).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("PhysicalBone", typeid(PhysicalBone).hash_code(), typeid(PhysicsBody).hash_code());
	godot::_TagDB::register_global_type("Physics2DDirectBodyState", typeid(Physics2DDirectBodyState).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("Physics2DDirectBodyStateSW", typeid(Physics2DDirectBodyStateSW).hash_code(), typeid(Physics2DDirectBodyState).hash_code());
	godot::_TagDB::register_global_type("Physics2DDirectSpaceState", typeid(Physics2DDirectSpaceState).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("Physics2DServer", typeid(Physics2DServer).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("Physics2DServerSW", typeid(Physics2DServerSW).hash_code(), typeid(Physics2DServer).hash_code());
	godot::_TagDB::register_global_type("Physics2DShapeQueryParameters", typeid(Physics2DShapeQueryParameters).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("Physics2DShapeQueryResult", typeid(Physics2DShapeQueryResult).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("Physics2DTestMotionResult", typeid(Physics2DTestMotionResult).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("PhysicsBody", typeid(PhysicsBody).hash_code(), typeid(CollisionObject).hash_code());
	godot::_TagDB::register_global_type("PhysicsBody2D", typeid(PhysicsBody2D).hash_code(), typeid(CollisionObject2D).hash_code());
	godot::_TagDB::register_global_type("PhysicsDirectBodyState", typeid(PhysicsDirectBodyState).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("PhysicsDirectSpaceState", typeid(PhysicsDirectSpaceState).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("PhysicsMaterial", typeid(PhysicsMaterial).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("PhysicsServer", typeid(PhysicsServer).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("PhysicsShapeQueryParameters", typeid(PhysicsShapeQueryParameters).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("PhysicsShapeQueryResult", typeid(PhysicsShapeQueryResult).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("PinJoint", typeid(PinJoint).hash_code(), typeid(Joint).hash_code());
	godot::_TagDB::register_global_type("PinJoint2D", typeid(PinJoint2D).hash_code(), typeid(Joint2D).hash_code());
	godot::_TagDB::register_global_type("PlaneMesh", typeid(PlaneMesh).hash_code(), typeid(PrimitiveMesh).hash_code());
	godot::_TagDB::register_global_type("PlaneShape", typeid(PlaneShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("PluginScript", typeid(PluginScript).hash_code(), typeid(Script).hash_code());
	godot::_TagDB::register_global_type("PointMesh", typeid(PointMesh).hash_code(), typeid(PrimitiveMesh).hash_code());
	godot::_TagDB::register_global_type("Polygon2D", typeid(Polygon2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("PolygonPathFinder", typeid(PolygonPathFinder).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Popup", typeid(Popup).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("PopupDialog", typeid(PopupDialog).hash_code(), typeid(Popup).hash_code());
	godot::_TagDB::register_global_type("PopupMenu", typeid(PopupMenu).hash_code(), typeid(Popup).hash_code());
	godot::_TagDB::register_global_type("PopupPanel", typeid(PopupPanel).hash_code(), typeid(Popup).hash_code());
	godot::_TagDB::register_global_type("Position2D", typeid(Position2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("Position3D", typeid(Position3D).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("PrimitiveMesh", typeid(PrimitiveMesh).hash_code(), typeid(Mesh).hash_code());
	godot::_TagDB::register_global_type("PrismMesh", typeid(PrismMesh).hash_code(), typeid(PrimitiveMesh).hash_code());
	godot::_TagDB::register_global_type("ProceduralSky", typeid(ProceduralSky).hash_code(), typeid(Sky).hash_code());
	godot::_TagDB::register_global_type("ProgressBar", typeid(ProgressBar).hash_code(), typeid(Range).hash_code());
	godot::_TagDB::register_global_type("ProjectSettings", typeid(ProjectSettings).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("ProximityGroup", typeid(ProximityGroup).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("ProxyTexture", typeid(ProxyTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("QuadMesh", typeid(QuadMesh).hash_code(), typeid(PrimitiveMesh).hash_code());
	godot::_TagDB::register_global_type("RandomNumberGenerator", typeid(RandomNumberGenerator).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("Range", typeid(Range).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("RayCast", typeid(RayCast).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("RayCast2D", typeid(RayCast2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("RayShape", typeid(RayShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("RayShape2D", typeid(RayShape2D).hash_code(), typeid(Shape2D).hash_code());
	godot::_TagDB::register_global_type("RectangleShape2D", typeid(RectangleShape2D).hash_code(), typeid(Shape2D).hash_code());
	godot::_TagDB::register_global_type("Reference", typeid(Reference).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("ReferenceRect", typeid(ReferenceRect).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("ReflectionProbe", typeid(ReflectionProbe).hash_code(), typeid(VisualInstance).hash_code());
	godot::_TagDB::register_global_type("RegEx", typeid(RegEx).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("RegExMatch", typeid(RegExMatch).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("RemoteTransform", typeid(RemoteTransform).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("RemoteTransform2D", typeid(RemoteTransform2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("Resource", typeid(Resource).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("ResourceFormatLoader", typeid(ResourceFormatLoader).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("ResourceFormatLoaderCrypto", typeid(ResourceFormatLoaderCrypto).hash_code(), typeid(ResourceFormatLoader).hash_code());
	godot::_TagDB::register_global_type("ResourceFormatSaver", typeid(ResourceFormatSaver).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("ResourceFormatSaverCrypto", typeid(ResourceFormatSaverCrypto).hash_code(), typeid(ResourceFormatSaver).hash_code());
	godot::_TagDB::register_global_type("ResourceImporter", typeid(ResourceImporter).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("ResourceInteractiveLoader", typeid(ResourceInteractiveLoader).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("ResourcePreloader", typeid(ResourcePreloader).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("RichTextEffect", typeid(RichTextEffect).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("RichTextLabel", typeid(RichTextLabel).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("RigidBody", typeid(RigidBody).hash_code(), typeid(PhysicsBody).hash_code());
	godot::_TagDB::register_global_type("RigidBody2D", typeid(RigidBody2D).hash_code(), typeid(PhysicsBody2D).hash_code());
	godot::_TagDB::register_global_type("RootMotionView", typeid(RootMotionView).hash_code(), typeid(VisualInstance).hash_code());
	godot::_TagDB::register_global_type("SceneState", typeid(SceneState).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("SceneTree", typeid(SceneTree).hash_code(), typeid(MainLoop).hash_code());
	godot::_TagDB::register_global_type("SceneTreeTimer", typeid(SceneTreeTimer).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("Script", typeid(Script).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("ScriptCreateDialog", typeid(ScriptCreateDialog).hash_code(), typeid(ConfirmationDialog).hash_code());
	godot::_TagDB::register_global_type("ScriptEditor", typeid(ScriptEditor).hash_code(), typeid(PanelContainer).hash_code());
	godot::_TagDB::register_global_type("ScrollBar", typeid(ScrollBar).hash_code(), typeid(Range).hash_code());
	godot::_TagDB::register_global_type("ScrollContainer", typeid(ScrollContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("SegmentShape2D", typeid(SegmentShape2D).hash_code(), typeid(Shape2D).hash_code());
	godot::_TagDB::register_global_type("Separator", typeid(Separator).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("Shader", typeid(Shader).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("ShaderMaterial", typeid(ShaderMaterial).hash_code(), typeid(Material).hash_code());
	godot::_TagDB::register_global_type("Shape", typeid(Shape).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Shape2D", typeid(Shape2D).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("ShortCut", typeid(ShortCut).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Skeleton", typeid(Skeleton).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("Skeleton2D", typeid(Skeleton2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("SkeletonIK", typeid(SkeletonIK).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("Skin", typeid(Skin).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("SkinReference", typeid(SkinReference).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("Sky", typeid(Sky).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Slider", typeid(Slider).hash_code(), typeid(Range).hash_code());
	godot::_TagDB::register_global_type("SliderJoint", typeid(SliderJoint).hash_code(), typeid(Joint).hash_code());
	godot::_TagDB::register_global_type("SoftBody", typeid(SoftBody).hash_code(), typeid(MeshInstance).hash_code());
	godot::_TagDB::register_global_type("Spatial", typeid(Spatial).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("SpatialGizmo", typeid(SpatialGizmo).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("SpatialMaterial", typeid(SpatialMaterial).hash_code(), typeid(Material).hash_code());
	godot::_TagDB::register_global_type("SpatialVelocityTracker", typeid(SpatialVelocityTracker).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("SphereMesh", typeid(SphereMesh).hash_code(), typeid(PrimitiveMesh).hash_code());
	godot::_TagDB::register_global_type("SphereShape", typeid(SphereShape).hash_code(), typeid(Shape).hash_code());
	godot::_TagDB::register_global_type("SpinBox", typeid(SpinBox).hash_code(), typeid(Range).hash_code());
	godot::_TagDB::register_global_type("SplitContainer", typeid(SplitContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("SpotLight", typeid(SpotLight).hash_code(), typeid(Light).hash_code());
	godot::_TagDB::register_global_type("SpringArm", typeid(SpringArm).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("Sprite", typeid(Sprite).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("Sprite3D", typeid(Sprite3D).hash_code(), typeid(SpriteBase3D).hash_code());
	godot::_TagDB::register_global_type("SpriteBase3D", typeid(SpriteBase3D).hash_code(), typeid(GeometryInstance).hash_code());
	godot::_TagDB::register_global_type("SpriteFrames", typeid(SpriteFrames).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("StaticBody", typeid(StaticBody).hash_code(), typeid(PhysicsBody).hash_code());
	godot::_TagDB::register_global_type("StaticBody2D", typeid(StaticBody2D).hash_code(), typeid(PhysicsBody2D).hash_code());
	godot::_TagDB::register_global_type("StreamPeer", typeid(StreamPeer).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("StreamPeerBuffer", typeid(StreamPeerBuffer).hash_code(), typeid(StreamPeer).hash_code());
	godot::_TagDB::register_global_type("StreamPeerGDNative", typeid(StreamPeerGDNative).hash_code(), typeid(StreamPeer).hash_code());
	godot::_TagDB::register_global_type("StreamPeerSSL", typeid(StreamPeerSSL).hash_code(), typeid(StreamPeer).hash_code());
	godot::_TagDB::register_global_type("StreamPeerTCP", typeid(StreamPeerTCP).hash_code(), typeid(StreamPeer).hash_code());
	godot::_TagDB::register_global_type("StreamTexture", typeid(StreamTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("StyleBox", typeid(StyleBox).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("StyleBoxEmpty", typeid(StyleBoxEmpty).hash_code(), typeid(StyleBox).hash_code());
	godot::_TagDB::register_global_type("StyleBoxFlat", typeid(StyleBoxFlat).hash_code(), typeid(StyleBox).hash_code());
	godot::_TagDB::register_global_type("StyleBoxLine", typeid(StyleBoxLine).hash_code(), typeid(StyleBox).hash_code());
	godot::_TagDB::register_global_type("StyleBoxTexture", typeid(StyleBoxTexture).hash_code(), typeid(StyleBox).hash_code());
	godot::_TagDB::register_global_type("SurfaceTool", typeid(SurfaceTool).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("TCP_Server", typeid(TCP_Server).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("TabContainer", typeid(TabContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("Tabs", typeid(Tabs).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("TextEdit", typeid(TextEdit).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("TextFile", typeid(TextFile).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Texture", typeid(Texture).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Texture3D", typeid(Texture3D).hash_code(), typeid(TextureLayered).hash_code());
	godot::_TagDB::register_global_type("TextureArray", typeid(TextureArray).hash_code(), typeid(TextureLayered).hash_code());
	godot::_TagDB::register_global_type("TextureButton", typeid(TextureButton).hash_code(), typeid(BaseButton).hash_code());
	godot::_TagDB::register_global_type("TextureLayered", typeid(TextureLayered).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("TextureProgress", typeid(TextureProgress).hash_code(), typeid(Range).hash_code());
	godot::_TagDB::register_global_type("TextureRect", typeid(TextureRect).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("Theme", typeid(Theme).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("TileMap", typeid(TileMap).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("TileSet", typeid(TileSet).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("Timer", typeid(Timer).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("ToolButton", typeid(ToolButton).hash_code(), typeid(Button).hash_code());
	godot::_TagDB::register_global_type("TouchScreenButton", typeid(TouchScreenButton).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("Translation", typeid(Translation).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("TranslationServer", typeid(TranslationServer).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("Tree", typeid(Tree).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("TreeItem", typeid(TreeItem).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("TriangleMesh", typeid(TriangleMesh).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("Tween", typeid(Tween).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("UPNP", typeid(UPNP).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("UPNPDevice", typeid(UPNPDevice).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("UndoRedo", typeid(UndoRedo).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("VBoxContainer", typeid(VBoxContainer).hash_code(), typeid(BoxContainer).hash_code());
	godot::_TagDB::register_global_type("VScrollBar", typeid(VScrollBar).hash_code(), typeid(ScrollBar).hash_code());
	godot::_TagDB::register_global_type("VSeparator", typeid(VSeparator).hash_code(), typeid(Separator).hash_code());
	godot::_TagDB::register_global_type("VSlider", typeid(VSlider).hash_code(), typeid(Slider).hash_code());
	godot::_TagDB::register_global_type("VSplitContainer", typeid(VSplitContainer).hash_code(), typeid(SplitContainer).hash_code());
	godot::_TagDB::register_global_type("VehicleBody", typeid(VehicleBody).hash_code(), typeid(RigidBody).hash_code());
	godot::_TagDB::register_global_type("VehicleWheel", typeid(VehicleWheel).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("VideoPlayer", typeid(VideoPlayer).hash_code(), typeid(Control).hash_code());
	godot::_TagDB::register_global_type("VideoStream", typeid(VideoStream).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("VideoStreamGDNative", typeid(VideoStreamGDNative).hash_code(), typeid(VideoStream).hash_code());
	godot::_TagDB::register_global_type("VideoStreamTheora", typeid(VideoStreamTheora).hash_code(), typeid(VideoStream).hash_code());
	godot::_TagDB::register_global_type("VideoStreamWebm", typeid(VideoStreamWebm).hash_code(), typeid(VideoStream).hash_code());
	godot::_TagDB::register_global_type("Viewport", typeid(Viewport).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("ViewportContainer", typeid(ViewportContainer).hash_code(), typeid(Container).hash_code());
	godot::_TagDB::register_global_type("ViewportTexture", typeid(ViewportTexture).hash_code(), typeid(Texture).hash_code());
	godot::_TagDB::register_global_type("VisibilityEnabler", typeid(VisibilityEnabler).hash_code(), typeid(VisibilityNotifier).hash_code());
	godot::_TagDB::register_global_type("VisibilityEnabler2D", typeid(VisibilityEnabler2D).hash_code(), typeid(VisibilityNotifier2D).hash_code());
	godot::_TagDB::register_global_type("VisibilityNotifier", typeid(VisibilityNotifier).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("VisibilityNotifier2D", typeid(VisibilityNotifier2D).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("VisualInstance", typeid(VisualInstance).hash_code(), typeid(Spatial).hash_code());
	godot::_TagDB::register_global_type("VisualScript", typeid(VisualScript).hash_code(), typeid(Script).hash_code());
	godot::_TagDB::register_global_type("VisualScriptBasicTypeConstant", typeid(VisualScriptBasicTypeConstant).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptBuiltinFunc", typeid(VisualScriptBuiltinFunc).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptClassConstant", typeid(VisualScriptClassConstant).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptComment", typeid(VisualScriptComment).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptComposeArray", typeid(VisualScriptComposeArray).hash_code(), typeid(VisualScriptLists).hash_code());
	godot::_TagDB::register_global_type("VisualScriptCondition", typeid(VisualScriptCondition).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptConstant", typeid(VisualScriptConstant).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptConstructor", typeid(VisualScriptConstructor).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptCustomNode", typeid(VisualScriptCustomNode).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptDeconstruct", typeid(VisualScriptDeconstruct).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptEmitSignal", typeid(VisualScriptEmitSignal).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptEngineSingleton", typeid(VisualScriptEngineSingleton).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptExpression", typeid(VisualScriptExpression).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptFunction", typeid(VisualScriptFunction).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptFunctionCall", typeid(VisualScriptFunctionCall).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptFunctionState", typeid(VisualScriptFunctionState).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("VisualScriptGlobalConstant", typeid(VisualScriptGlobalConstant).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptIndexGet", typeid(VisualScriptIndexGet).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptIndexSet", typeid(VisualScriptIndexSet).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptInputAction", typeid(VisualScriptInputAction).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptIterator", typeid(VisualScriptIterator).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptLists", typeid(VisualScriptLists).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptLocalVar", typeid(VisualScriptLocalVar).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptLocalVarSet", typeid(VisualScriptLocalVarSet).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptMathConstant", typeid(VisualScriptMathConstant).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptNode", typeid(VisualScriptNode).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("VisualScriptOperator", typeid(VisualScriptOperator).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptPreload", typeid(VisualScriptPreload).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptPropertyGet", typeid(VisualScriptPropertyGet).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptPropertySet", typeid(VisualScriptPropertySet).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptResourcePath", typeid(VisualScriptResourcePath).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptReturn", typeid(VisualScriptReturn).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptSceneNode", typeid(VisualScriptSceneNode).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptSceneTree", typeid(VisualScriptSceneTree).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptSelect", typeid(VisualScriptSelect).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptSelf", typeid(VisualScriptSelf).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptSequence", typeid(VisualScriptSequence).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptSubCall", typeid(VisualScriptSubCall).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptSwitch", typeid(VisualScriptSwitch).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptTypeCast", typeid(VisualScriptTypeCast).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptVariableGet", typeid(VisualScriptVariableGet).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptVariableSet", typeid(VisualScriptVariableSet).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptWhile", typeid(VisualScriptWhile).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptYield", typeid(VisualScriptYield).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualScriptYieldSignal", typeid(VisualScriptYieldSignal).hash_code(), typeid(VisualScriptNode).hash_code());
	godot::_TagDB::register_global_type("VisualServer", typeid(VisualServer).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("VisualShader", typeid(VisualShader).hash_code(), typeid(Shader).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNode", typeid(VisualShaderNode).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeBooleanConstant", typeid(VisualShaderNodeBooleanConstant).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeBooleanUniform", typeid(VisualShaderNodeBooleanUniform).hash_code(), typeid(VisualShaderNodeUniform).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeColorConstant", typeid(VisualShaderNodeColorConstant).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeColorFunc", typeid(VisualShaderNodeColorFunc).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeColorOp", typeid(VisualShaderNodeColorOp).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeColorUniform", typeid(VisualShaderNodeColorUniform).hash_code(), typeid(VisualShaderNodeUniform).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeCompare", typeid(VisualShaderNodeCompare).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeCubeMap", typeid(VisualShaderNodeCubeMap).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeCubeMapUniform", typeid(VisualShaderNodeCubeMapUniform).hash_code(), typeid(VisualShaderNodeTextureUniform).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeCustom", typeid(VisualShaderNodeCustom).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeDeterminant", typeid(VisualShaderNodeDeterminant).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeDotProduct", typeid(VisualShaderNodeDotProduct).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeExpression", typeid(VisualShaderNodeExpression).hash_code(), typeid(VisualShaderNodeGroupBase).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeFaceForward", typeid(VisualShaderNodeFaceForward).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeFresnel", typeid(VisualShaderNodeFresnel).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeGlobalExpression", typeid(VisualShaderNodeGlobalExpression).hash_code(), typeid(VisualShaderNodeExpression).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeGroupBase", typeid(VisualShaderNodeGroupBase).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeIf", typeid(VisualShaderNodeIf).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeInput", typeid(VisualShaderNodeInput).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeIs", typeid(VisualShaderNodeIs).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeOuterProduct", typeid(VisualShaderNodeOuterProduct).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeOutput", typeid(VisualShaderNodeOutput).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarClamp", typeid(VisualShaderNodeScalarClamp).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarConstant", typeid(VisualShaderNodeScalarConstant).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarDerivativeFunc", typeid(VisualShaderNodeScalarDerivativeFunc).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarFunc", typeid(VisualShaderNodeScalarFunc).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarInterp", typeid(VisualShaderNodeScalarInterp).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarOp", typeid(VisualShaderNodeScalarOp).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarSmoothStep", typeid(VisualShaderNodeScalarSmoothStep).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarSwitch", typeid(VisualShaderNodeScalarSwitch).hash_code(), typeid(VisualShaderNodeSwitch).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeScalarUniform", typeid(VisualShaderNodeScalarUniform).hash_code(), typeid(VisualShaderNodeUniform).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeSwitch", typeid(VisualShaderNodeSwitch).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTexture", typeid(VisualShaderNodeTexture).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTextureUniform", typeid(VisualShaderNodeTextureUniform).hash_code(), typeid(VisualShaderNodeUniform).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTextureUniformTriplanar", typeid(VisualShaderNodeTextureUniformTriplanar).hash_code(), typeid(VisualShaderNodeTextureUniform).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTransformCompose", typeid(VisualShaderNodeTransformCompose).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTransformConstant", typeid(VisualShaderNodeTransformConstant).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTransformDecompose", typeid(VisualShaderNodeTransformDecompose).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTransformFunc", typeid(VisualShaderNodeTransformFunc).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTransformMult", typeid(VisualShaderNodeTransformMult).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTransformUniform", typeid(VisualShaderNodeTransformUniform).hash_code(), typeid(VisualShaderNodeUniform).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeTransformVecMult", typeid(VisualShaderNodeTransformVecMult).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeUniform", typeid(VisualShaderNodeUniform).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVec3Constant", typeid(VisualShaderNodeVec3Constant).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVec3Uniform", typeid(VisualShaderNodeVec3Uniform).hash_code(), typeid(VisualShaderNodeUniform).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorClamp", typeid(VisualShaderNodeVectorClamp).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorCompose", typeid(VisualShaderNodeVectorCompose).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorDecompose", typeid(VisualShaderNodeVectorDecompose).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorDerivativeFunc", typeid(VisualShaderNodeVectorDerivativeFunc).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorDistance", typeid(VisualShaderNodeVectorDistance).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorFunc", typeid(VisualShaderNodeVectorFunc).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorInterp", typeid(VisualShaderNodeVectorInterp).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorLen", typeid(VisualShaderNodeVectorLen).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorOp", typeid(VisualShaderNodeVectorOp).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorRefract", typeid(VisualShaderNodeVectorRefract).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorScalarMix", typeid(VisualShaderNodeVectorScalarMix).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorScalarSmoothStep", typeid(VisualShaderNodeVectorScalarSmoothStep).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorScalarStep", typeid(VisualShaderNodeVectorScalarStep).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("VisualShaderNodeVectorSmoothStep", typeid(VisualShaderNodeVectorSmoothStep).hash_code(), typeid(VisualShaderNode).hash_code());
	godot::_TagDB::register_global_type("WeakRef", typeid(WeakRef).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("WebRTCDataChannel", typeid(WebRTCDataChannel).hash_code(), typeid(PacketPeer).hash_code());
	godot::_TagDB::register_global_type("WebRTCDataChannelGDNative", typeid(WebRTCDataChannelGDNative).hash_code(), typeid(WebRTCDataChannel).hash_code());
	godot::_TagDB::register_global_type("WebRTCMultiplayer", typeid(WebRTCMultiplayer).hash_code(), typeid(NetworkedMultiplayerPeer).hash_code());
	godot::_TagDB::register_global_type("WebRTCPeerConnection", typeid(WebRTCPeerConnection).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("WebRTCPeerConnectionGDNative", typeid(WebRTCPeerConnectionGDNative).hash_code(), typeid(WebRTCPeerConnection).hash_code());
	godot::_TagDB::register_global_type("WebSocketClient", typeid(WebSocketClient).hash_code(), typeid(WebSocketMultiplayerPeer).hash_code());
	godot::_TagDB::register_global_type("WebSocketMultiplayerPeer", typeid(WebSocketMultiplayerPeer).hash_code(), typeid(NetworkedMultiplayerPeer).hash_code());
	godot::_TagDB::register_global_type("WebSocketPeer", typeid(WebSocketPeer).hash_code(), typeid(PacketPeer).hash_code());
	godot::_TagDB::register_global_type("WebSocketServer", typeid(WebSocketServer).hash_code(), typeid(WebSocketMultiplayerPeer).hash_code());
	godot::_TagDB::register_global_type("WindowDialog", typeid(WindowDialog).hash_code(), typeid(Popup).hash_code());
	godot::_TagDB::register_global_type("World", typeid(World).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("World2D", typeid(World2D).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("WorldEnvironment", typeid(WorldEnvironment).hash_code(), typeid(Node).hash_code());
	godot::_TagDB::register_global_type("X509Certificate", typeid(X509Certificate).hash_code(), typeid(Resource).hash_code());
	godot::_TagDB::register_global_type("XMLParser", typeid(XMLParser).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("YSort", typeid(YSort).hash_code(), typeid(Node2D).hash_code());
	godot::_TagDB::register_global_type("_ClassDB", typeid(ClassDB).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("_Directory", typeid(Directory).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("_Engine", typeid(Engine).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("_File", typeid(File).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("_Geometry", typeid(Geometry).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("_JSON", typeid(JSON).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("_Marshalls", typeid(Marshalls).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("_Mutex", typeid(Mutex).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("_OS", typeid(OS).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("_ResourceLoader", typeid(ResourceLoader).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("_ResourceSaver", typeid(ResourceSaver).hash_code(), typeid(Object).hash_code());
	godot::_TagDB::register_global_type("_Semaphore", typeid(Semaphore).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("_Thread", typeid(Thread).hash_code(), typeid(Reference).hash_code());
	godot::_TagDB::register_global_type("_VisualScriptEditor", typeid(VisualScriptEditor).hash_code(), typeid(Object).hash_code());
}

}