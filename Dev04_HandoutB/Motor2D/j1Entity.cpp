#include "j1Entity.h"
#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Audio.h"
#include "j1Textures.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1Collider.h"
#include "j1EntityManager.h"
#include "j1Player.h"

j1Entity::j1Entity(entity_type type) : type(type){
}

j1Entity::~j1Entity()
{
	
}

bool j1Entity::CleanUp() {

	entity_colliders->to_delete = true;
	return true;

}

bool j1Entity::Load(pugi::xml_node& data) {
	return true;
}

bool j1Entity::Save(pugi::xml_node& data) const {
	return true;
}


void j1Entity::DrawCollider() {

	entity_colliders->SetPos(position.x, position.y);

}
