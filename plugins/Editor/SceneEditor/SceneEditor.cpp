//-----------------------------------------------------------------------------
// Copyright (c) 2015 Andrew Mac
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "SceneEditor.h"
#include <sim/simObject.h>
#include <plugins/plugins_shared.h>
#include <3d/rendering/common.h>
#include <graphics/utilities.h>
#include <bx/fpumath.h>

using namespace Plugins;

SceneEditor sceneEditor;

IMPLEMENT_PLUGIN_CONOBJECT(SceneEditorCamera);

SceneEditorCamera::SceneEditorCamera()
{
   lastMousePosition.set(0, 0);
   mouseDirection.set(0.0f, 0.0f);
   translateDirection.set(0.0f, 0.0f, 0.0f);
}

void SceneEditorCamera::onMouseDownEvent(const GuiEvent &event)
{
   Point3F world_ray = Link.Rendering.screenToWorld(event.mousePoint);

   Scene::SceneEntity* hit_entity = NULL;
   if ( gizmoVisible && sceneEditor.mSelectedEntity != NULL )
   {
      //Con::printf("Mouse Position: %d %d", event.mousePoint.x, event.mousePoint.y);
      //Con::printf("Gizmo Min: %d %d Max: %d %d", gizmoMinCoord.x, gizmoMinCoord.y, gizmoMaxCoord.x, gizmoMaxCoord.y);
      if ( event.mousePoint.x >= gizmoMinCoord.x && event.mousePoint.y >= gizmoMinCoord.y
           && event.mousePoint.x <= gizmoMaxCoord.x && event.mousePoint.y <= gizmoMaxCoord.y )
      {
         hit_entity = sceneEditor.mSelectedEntity;
      }
   } 

   if ( hit_entity == NULL )
      hit_entity = Link.Scene.raycast(mPosition, mPosition + (world_ray * 1000.0f));
   
   if ( hit_entity )
   {
      translateDirection.set(0,0,0);
      lastMousePosition = event.mousePoint;

      if ( sceneEditor.mSelectedEntity == hit_entity ) 
      {
         Point2I entityScreenPos = Link.Rendering.worldToScreen(sceneEditor.mSelectedEntity->mPosition);
         Point2I redScreenPos = Link.Rendering.worldToScreen(sceneEditor.mSelectedEntity->mPosition + Point3F(5, 0, 0));
         Point2I greenScreenPos = Link.Rendering.worldToScreen(sceneEditor.mSelectedEntity->mPosition + Point3F(0, 5, 0));
         Point2I blueScreenPos = Link.Rendering.worldToScreen(sceneEditor.mSelectedEntity->mPosition + Point3F(0, 0, 5));
         Point2I redScreenPosEnd = Link.Rendering.worldToScreen(sceneEditor.mSelectedEntity->mPosition + Point3F(50, 0, 0));
         Point2I greenScreenPosEnd = Link.Rendering.worldToScreen(sceneEditor.mSelectedEntity->mPosition + Point3F(0, 50, 0));
         Point2I blueScreenPosEnd = Link.Rendering.worldToScreen(sceneEditor.mSelectedEntity->mPosition + Point3F(0, 0, 50));

         gizmoVisible = true;

         gizmoMinCoord = entityScreenPos;
         gizmoMinCoord.setMin(redScreenPosEnd);
         gizmoMinCoord.setMin(greenScreenPosEnd);
         gizmoMinCoord.setMin(blueScreenPosEnd);

         gizmoMaxCoord = entityScreenPos;
         gizmoMaxCoord.setMax(redScreenPosEnd);
         gizmoMaxCoord.setMax(greenScreenPosEnd);
         gizmoMaxCoord.setMax(blueScreenPosEnd);

         F32 entityDist = Point2I(entityScreenPos - event.mousePoint).len();
         F32 redDist = Point2I(redScreenPos - event.mousePoint).len();
         F32 greenDist = Point2I(greenScreenPos - event.mousePoint).len();
         F32 blueDist = Point2I(blueScreenPos - event.mousePoint).len();
         F32 redDistEnd = Point2I(redScreenPosEnd - event.mousePoint).len();
         F32 greenDistEnd = Point2I(greenScreenPosEnd - event.mousePoint).len();
         F32 blueDistEnd = Point2I(blueScreenPosEnd - event.mousePoint).len();

         if (( redDist < greenDist && redDist < blueDist ) || ( redDistEnd < greenDist && redDistEnd < blueDist ))
         {
            translateDirection.set(1.0f, 0.0f, 0.0f);
            mouseDirection.set(redScreenPos.x - entityScreenPos.x, redScreenPos.y - entityScreenPos.y);
            mouseDirection.normalize();
         }
         else if (( greenDist < redDist && greenDist < blueDist ) || ( greenDistEnd < redDist && greenDistEnd < blueDist ))
         {
            translateDirection.set(0.0f, 1.0f, 0.0f);
            mouseDirection.set(greenScreenPos.x - entityScreenPos.x, greenScreenPos.y - entityScreenPos.y);
            mouseDirection.normalize();
         }
         else if (( blueDist < redDist && blueDist < greenDist ) || ( blueDistEnd < redDist && blueDistEnd < greenDist ))
         {
            translateDirection.set(0.0f, 0.0f, 1.0f);
            mouseDirection.set(blueScreenPos.x - entityScreenPos.x, blueScreenPos.y - entityScreenPos.y);
            mouseDirection.normalize();
         }
      } else {
         sceneEditor.selectEntity(hit_entity);
      }
   } else {
      // Didn't hit anything? Clear selection.
      sceneEditor.clearSelection();
      gizmoVisible = false;
   }
}

void SceneEditorCamera::onMouseDraggedEvent(const GuiEvent &event)
{
   Point2F newDirection(event.mousePoint.x - lastMousePosition.x, event.mousePoint.y - lastMousePosition.y);
   F32 mouseDist = newDirection.len();
   newDirection.normalize();
   F32 dir_dist = Point2F(mouseDirection - newDirection).len();
   
   F32 dist = 0.0f;
   if ( dir_dist < 0.3f )
      dist = mouseDist;
   if ( dir_dist > 1.7f )
      dist = -mouseDist;

   if ( sceneEditor.mSelectedEntity != NULL )
   {
      sceneEditor.mSelectedEntity->mPosition += translateDirection * (dist);
      sceneEditor.mSelectedEntity->refresh();
      lastMousePosition = event.mousePoint;
      sceneEditor.selectEntity(sceneEditor.mSelectedEntity);
   }
}

SceneEditor::SceneEditor()
{
   mSceneGroup = NULL;
   mSelectedEntity = NULL;

   dirLightDiffuse = -1;
   dirLightAmbient = -1;
   sceneEditorArea = -1;
   sceneOverviewArea = -1;
   sceneOverviewList = -1;
   entityInspectorArea = -1;

   addEntityArea = -1;
   addEntityOpen = false;
   addEntityList = -1;
}

SceneEditor::~SceneEditor()
{

}

void SceneEditor::enable()
{
   if ( sceneEditorArea < 0 )
   {
      // Scene Editor
      sceneEditorArea = Link.SysGUI.beginScrollArea("Scene Editor", 5, 160, 300, 600);
      Link.SysGUI.separator();
      Link.SysGUI.button("Load Scene", "", NULL);
      Link.SysGUI.button("Save Scene", "", NULL);
      Link.SysGUI.separator();
      Link.SysGUI.label("Camera Settings");
      Link.SysGUI.separator();
      Link.SysGUI.slider("Speed", 50, 0, 100);
      Link.SysGUI.button("Return to Center", "", NULL);

      Link.SysGUI.separator();
      Link.SysGUI.beginCollapse("Directional Light", "", false);
      Link.SysGUI.beginCollapse("Direction", "[1, 0, 0]", false);
      Link.SysGUI.endCollapse();
      dirLightDiffuse = Link.SysGUI.colorWheel("Diffuse", *Link.Scene.directionalLightColor);
      dirLightAmbient = Link.SysGUI.colorWheel("Ambient", *Link.Scene.directionalLightAmbient);
      Link.SysGUI.endCollapse();
      Link.SysGUI.separator();

      Link.SysGUI.endScrollArea();
   
      // Scene Overview
      sceneOverviewArea = Link.SysGUI.beginScrollArea("Scene Overview", 5, 5, 300, 300);
      Link.SysGUI.separator();
      Link.SysGUI.button("Add Entity", "", clickAddEntity);
      Link.SysGUI.separator();
      sceneOverviewList = Link.SysGUI.list("", NULL);
      Link.SysGUI.endScrollArea();
      Link.SysGUI.alignRight(sceneOverviewArea);

      // Entity Inspector
      entityInspectorArea = Link.SysGUI.beginScrollArea("Entity Inspector", 5, 310, 300, 400);
      Link.SysGUI.alignRight(entityInspectorArea);
      Link.SysGUI.endScrollArea();

      addEntityArea = Link.SysGUI.beginScrollArea("Add Entity", 310, 5, 200, 400);
      Link.SysGUI.alignRight(addEntityArea);
      Link.SysGUI.separator();
      addEntityList = Link.SysGUI.list("", clickAddEntityList);
      Link.SysGUI.separator();
      Link.SysGUI.endScrollArea();
      Link.SysGUI.setElementHidden(addEntityArea, true);

      mCamera.setBindMouse(true, false, true);
      Link.Scene.addCamera("EditorCamera", &mCamera);

      mSceneGroup = Link.Scene.getEntityGroup();
   }

   Link.SysGUI.setElementHidden(sceneEditorArea, false);
   Link.SysGUI.setElementHidden(sceneOverviewArea, false);
   Link.SysGUI.setElementHidden(entityInspectorArea, false);
   if ( addEntityOpen )
      Link.SysGUI.setElementHidden(addEntityArea, false);
   Link.Scene.pushActiveCamera("EditorCamera");
   setProcessTicks(true);
}

void SceneEditor::disable()
{
   Link.SysGUI.setElementHidden(sceneEditorArea, true);
   Link.SysGUI.setElementHidden(sceneOverviewArea, true);
   Link.SysGUI.setElementHidden(entityInspectorArea, true);
   Link.SysGUI.setElementHidden(addEntityArea, true);
   Link.Scene.popActiveCamera();
   setProcessTicks(false);
}

void SceneEditor::render()
{
   if ( mSelectedEntity != NULL )
   {
      Link.Graphics.drawBox3D(mSelectedEntity->mBoundingBox, ColorI(255, 255, 255, 255), 2.0f);

      Link.Graphics.drawLine3D(mSelectedEntity->mPosition, mSelectedEntity->mPosition + Point3F(0, 50, 0), ColorI(0, 200, 0, 255), 5.0f);
      Link.Graphics.drawLine3D(mSelectedEntity->mPosition, mSelectedEntity->mPosition + Point3F(50, 0, 0), ColorI(200, 0, 0, 255), 5.0f);
      Link.Graphics.drawLine3D(mSelectedEntity->mPosition, mSelectedEntity->mPosition + Point3F(0, 0, 50), ColorI(0, 0, 200, 255), 5.0f);
   }
}

void SceneEditor::selectEntity(Scene::SceneEntity* entity)
{
   clearSelection();
   mSelectedEntity = entity;
   Link.SysGUI.seek(entityInspectorArea);

   AbstractClassRep::FieldList fieldList = entity->getFieldList();
   for(Vector<AbstractClassRep::Field>::iterator itr = fieldList.begin(); itr != fieldList.end(); itr++)
   {
      const AbstractClassRep::Field* f = itr;
      if( f->type == AbstractClassRep::DepricatedFieldType ||
            f->type == AbstractClassRep::StartGroupFieldType ||
            f->type == AbstractClassRep::EndGroupFieldType) 
         continue;

      for(U32 j = 0; S32(j) < f->elementCount; j++)
      {                      
         const char *val = (*f->getDataFn)( entity, Link.Con.getData(f->type, (void *) (((const char *)entity) + f->offset), j, f->table, f->flag) );

         if( !val )
            continue;

         if ( f->type ==  Link.Con.TypeBool )
            Link.SysGUI.checkBox(f->pFieldname, dAtob(val));
         else if ( f->type ==  Link.Con.TypeString )
            Link.SysGUI.textInput(f->pFieldname, val);
         else if ( f->type ==  Link.Con.TypePoint3F )
         {
            char textValue[256];
            Point3F *pt = (Point3F *)(void *) (((const char *)entity) + f->offset);

            Link.SysGUI.vector3(f->pFieldname, *pt, "", changedVector3Field);
         }
      }
   }

   Link.SysGUI.clearSeek();
}

void SceneEditor::clearSelection()
{
   Link.SysGUI.clearScrollArea(entityInspectorArea);
   mSelectedEntity = NULL;
}

void SceneEditor::deleteKey()
{
   if ( mSelectedEntity != NULL )
   {
      Link.Scene.removeEntity(mSelectedEntity);
      clearSelection();
   }
}

void SceneEditor::processTick()
{
   if ( mSceneGroup != NULL )
   {
      Link.SysGUI.clearList(sceneOverviewList);
      for(U32 n = 0; n < mSceneGroup->size(); ++n)
      {
         SimObject* obj = mSceneGroup->at(n);
         Link.SysGUI.addListValue(sceneOverviewList, obj->getName(), "", clickOverviewList);
      }
   }

   // Directional Light: Diffuse
   ColorF dirLightColor = Link.SysGUI.getColorValue(dirLightDiffuse);
   if ( *Link.Scene.directionalLightColor != dirLightColor )
      Link.Scene.setDirectionalLight(*Link.Scene.directionalLightDir, dirLightColor, *Link.Scene.directionalLightAmbient);

   // Directional Light: Ambient
   ColorF dirLightAmbientColor = Link.SysGUI.getColorValue(dirLightAmbient);
   if ( *Link.Scene.directionalLightAmbient != dirLightAmbient )
      Link.Scene.setDirectionalLight(*Link.Scene.directionalLightDir, *Link.Scene.directionalLightColor, dirLightAmbientColor);
}

void SceneEditor::advanceTime(F32 delta)
{
   //
}

void SceneEditor::interpolateTick(F32 delta)
{
   //
}

void SceneEditor::toggleAddEntity()
{
   addEntityOpen = !addEntityOpen;
   Link.SysGUI.setElementHidden(addEntityArea, !addEntityOpen);
}

void clickOverviewList(S32 id)
{
   S32 selected_item = Link.SysGUI.getListSelected(sceneEditor.sceneOverviewList);
   if ( selected_item > -1 )
   {
      Scene::SceneEntity* entity = dynamic_cast<Scene::SceneEntity*>(sceneEditor.mSceneGroup->at(selected_item));
      if ( entity )
         sceneEditor.selectEntity(entity);
   }
}

void clickAddEntity(S32 id)
{
   Link.SysGUI.clearList(sceneEditor.addEntityList);

   bool inCategory = false;
   char last_mod_name[256];
   AssetQuery assQuery;
   Link.AssetDatabaseLink.findAssetType(&assQuery, "EntityTemplateAsset", false);
   for ( U32 n = 0; n < assQuery.size(); n++)
   {
      StringTableEntry assetID = assQuery[n];
      char buf[256];
      dStrcpy(buf, assetID);

      char* mod_name = dStrtok(buf, ":");
      if ( dStrcmp(last_mod_name, mod_name) != 0 )
      {
         dStrcpy(last_mod_name, mod_name);
         //if ( inCategory )
            //Link.SysGUI.endCollapse();
         inCategory = true;
         //Link.SysGUI.beginCollapse(mod_name, "", false);
         //Link.SysGUI.label(mod_name);
      }

      char* asset_name = dStrtok(NULL, ":");
      Link.SysGUI.addListValue(sceneEditor.addEntityList, assetID, "", NULL);
   }

   sceneEditor.toggleAddEntity();
}

void clickAddEntityList(S32 id)
{
   S32 selectedItem = Link.SysGUI.getListSelected(sceneEditor.addEntityList);
   const char* selected_value = Link.SysGUI.getListValue(sceneEditor.addEntityList, selectedItem);

   Scene::SceneEntity* entity = new Scene::SceneEntity();
   entity->setTemplateAsset(Link.StringTableLink->insert(selected_value));
   entity->mScale.set(10, 10, 10);
   Link.Scene.addEntity(entity, "MenuEntity");

   sceneEditor.toggleAddEntity();
}

void changedVector3Field(S32 id)
{
   const char* fieldName = Link.SysGUI.getLabelValue(id);
   Point3F fieldValue = Link.SysGUI.getVector3Value(id);
   if ( sceneEditor.mSelectedEntity != NULL )
   {
      sceneEditor.mSelectedEntity->setField(fieldName,  Link.Con.getData(Link.Con.TypePoint3F, fieldValue, 0, NULL, 0));
   }
}