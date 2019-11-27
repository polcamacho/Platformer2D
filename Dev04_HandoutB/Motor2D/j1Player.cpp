#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Render.h"
#include "j1Audio.h"
#include "j1Textures.h"
#include "j1Player.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1Input.h"
#include "j1Collider.h"
#include "j1Scene.h"
#include "j1EntityManager.h"
#include <math.h>
#include "..//Brofiler/Brofiler.h"


j1Player::j1Player(int x, int y):j1Entity(entity_type::PLAYER)
{
	name.create("player");
	
}

// Destructor
j1Player::~j1Player()
{}

bool j1Player::Awake(pugi::xml_node& config) {

	bool ret = true;

	//Load All Player Features from Config

	folder.create(config.child("folder").child_value());	
	texture = config.child("texture").attribute("source").as_string();

	data_entity.position.x = config.child("position").attribute("x").as_int();
	data_entity.position.y = config.child("position").attribute("y").as_int();

	data_player.jumpvel = config.child("jump_velocity").attribute("jumpvel").as_int();
	   
	data_entity.v.x = config.child("velocity").attribute("x").as_int();
	data_player.velrun = config.child("velrun").attribute("x").as_float();
	
	data_player.colOffset.x = config.child("colOffset").attribute("x").as_int();
	data_player.colOffset.y = config.child("colOffset").attribute("y").as_int();
	
	data_entity.gravity = config.child("gravity").attribute("grav").as_int();

	

	return ret;

}

void j1Player::DrawPlayer()
{
	
	BROFILER_CATEGORY("DrawPlayer", Profiler::Color::Orange);

	if (data_entity.flip) {
		App->render->Blit(App->entity->Tex_Player, data_entity.position.x, data_entity.position.y, &(data_entity.current_animation->GetCurrentFrame()), SDL_FLIP_HORIZONTAL, 0);	//Draw Player Flipped
	}
	else {
		App->render->Blit(App->entity->Tex_Player, data_entity.position.x, data_entity.position.y, &(data_entity.current_animation->GetCurrentFrame()), SDL_FLIP_NONE, 0);	//Draw Player Normal
	}


}

bool j1Player::Start() {

	data_entity.pretimer = 0;
	//globaltime = SDL_GetTicks();	//Sets the Global time to the death timer
	
	Pushbacks();	//Call all the Pushback of animations
	
	App->entity->Tex_Player = App->tex->Load(PATH(folder.GetString(), texture.GetString()));	//Load The Texture of player
	
	SDL_Rect c;
	c.x = data_entity.position.x;
	c.y = data_entity.position.y;
	c.w = 39;
	c.h = 53;

	data_entity.entity_colliders = App->collider->AddCollider(&c, COLLIDER_PLAYER, this);	//Sets The Collider Type and Dimensions to Player
	
	return	true;

}

bool j1Player::PreUpdate(float dt) {

	BROFILER_CATEGORY("PreUpdatePlayer", Profiler::Color::Gray);

	return true;

}

bool j1Player::Update(float dt) {

	BROFILER_CATEGORY("DrawPlayer", Profiler::Color::Red);


	if (App->input->GetKey(SDL_SCANCODE_F10) == KEY_DOWN) {

		if (godmode == false)	//If godmode is false sets None Collider to player for he can fly around map and not collide
		{

			data_entity.gravity = 0;	//Sets new gravity to player for he can move around map
			godmode = true;	

		}
		else if (godmode == true)
		{

			data_entity.gravity = 20;	//Sets normal gravity to player 
			godmode = false;
			
		}
	}
	
	if (godmode == true) {	//Sets the Debug keys to player move when he cant die

		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT)
			data_entity.position.y -= (int)(20 * dt * LIMIT_TIMER);

		if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT)
			data_entity.position.y += (int)(20 * dt * LIMIT_TIMER);

		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
			data_entity.position.x -= (int)(20 * dt * LIMIT_TIMER);

		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
			data_entity.position.x += (int)(20 * dt * LIMIT_TIMER);
	}
	
	data_entity.position.y += data_entity.gravity;
	data_entity.preposition = data_entity.position;
	
	CheckState(dt);	//Checks the state where is the player
	State(dt);	//Set the animation relationed with the state that he is
	
	//Player Collider Draw
	data_entity.entity_colliders->SetPos(data_entity.position.x, data_entity.position.y);	//Sets the Player Collider Position


	//Player Draw
	if (data_entity.flip) {
		App->render->Blit(App->entity->Tex_Player, data_entity.position.x, data_entity.position.y, &(data_entity.current_animation->GetCurrentFrame()), SDL_FLIP_HORIZONTAL, 1.0);	//Draw Player Flipped
	}
	else {
		App->render->Blit(App->entity->Tex_Player, data_entity.position.x, data_entity.position.y, &(data_entity.current_animation->GetCurrentFrame()), SDL_FLIP_NONE, 1.0);	//Draw Player Normal
	}

	if (data_entity.grounded == false) {	//Sets that if the player is not touching the ground puts the fall animation

		current_state = JUMP_FALL;

	}
		
	return true;

}

bool j1Player::PostUpdate(float dt) {

	return true;
}

// Called before quitting
bool j1Player::CleanUp()
{
	LOG("Unloading player");

	App->tex->UnLoad(App->entity->Tex_Player);	//Unload The Player texture
	App->collider->CleanUp();	//Unload the Player collider
	
	return true;
}

bool j1Player::Load(pugi::xml_node& node) {

	if (!node.child("position").empty())	//Load the valors if save_confign file is empty link
	{

		data_entity.position.x = node.child("position").attribute("x").as_int();	//Load Player X
		data_entity.position.y = node.child("position").attribute("y").as_int();	//Load Player Y
	}

	else {	
		data_entity.position.x = data_entity.position.x;	//Load Player X
		data_entity.position.y = data_entity.position.y;	//Load Player Y
	}
	return true;

}
bool j1Player::Save(pugi::xml_node& node) const {

	if (node.child("position").empty())	//Save The Position Valors in the save_config file
	{
		pugi::xml_node&  save = node.append_child("position");	
		save.append_attribute("x").set_value(data_entity.position.x);	//Save Player X
		save.append_attribute("y").set_value(data_entity.position.y);	//Save Player Y
	}
	else
	{
		node.child("position").attribute("x").set_value(data_entity.position.x);	//Save Player X
		node.child("position").attribute("y").set_value(data_entity.position.y);	//Save Player Y
	}
	
	return true;

}

void j1Player::Pushbacks() {

	//Sets the Idle Animation
	data_entity.idle.PushBack({ 7,554,48,52 }, 0.1, 0, 0);
	data_entity.idle.PushBack({ 84,554,41,52 }, 0.1, 10, 0);
	data_entity.idle.PushBack({ 156,555,42,50 }, 0.2, 10, 0);
	data_entity.idle.PushBack({ 232,557,41,48 }, 0.2, 10, 0);
	data_entity.idle.PushBack({ 291,557,41,48 }, 0.2, 10, 0);
	data_entity.idle.PushBack({ 351,557,40,48 }, 0.2, 10, 0);
	data_entity.idle.PushBack({ 415,556,41,49 }, 0.2, 10, 0);
	data_entity.idle.PushBack({ 474,556,42,49 }, 0.2, 10, 0);
	data_entity.idle.PushBack({ 533,557,41,48 }, 0.2, 10, 0);
	data_entity.idle.PushBack({ 591,554,41,51 }, 0.1, 0, 0);
	data_entity.idle.PushBack({ 643,554,52,52 }, 0.1, -1, 0);
	data_entity.idle.PushBack({ 705,555,44,50 }, 0.1, -6, 0);
	data_entity.idle.PushBack({ 763,554,43,52 }, 0.2, -20, 0);
	data_entity.idle.PushBack({ 819,554,45,52 }, 0.2, -20, 0);
	data_entity.idle.PushBack({ 880,554,43,51 }, 0.2, -20, 0);
	data_entity.idle.PushBack({ 938,555,43,51 }, 0.2, -20, 0);
	data_entity.idle.PushBack({ 995,554,44,52 }, 0.2, -20, 0);
	data_entity.idle.loop = true;

	//Sets the Walk Animation
	data_entity.walk.PushBack({ 7,4,42,49 }, 0.2, 0, 0);
	data_entity.walk.PushBack({ 65,4,41,49 }, 0.2, 0, 0);
	data_entity.walk.PushBack({ 131,3,38,50 }, 0.2, 0, 0);
	data_entity.walk.PushBack({ 201,3,36,51 }, 0.2, 0, 0);
	data_entity.walk.PushBack({ 259,5,38,48 }, 0.2, 0, 0);
	data_entity.walk.PushBack({ 317,4,44,50 }, 0.2, 0, 0);
	data_entity.walk.loop = true;

	//Sets the Run Animation
	data_player.walk2.PushBack({ 7,4,42,49 }, 0.4, 0, 0);
	data_player.walk2.PushBack({ 65,4,41,49 }, 0.4, 0, 0);
	data_player.walk2.PushBack({ 131,3,38,50 }, 0.4, 0, 0);
	data_player.walk2.PushBack({ 201,3,36,51 }, 0.4, 0, 0);
	data_player.walk2.PushBack({ 259,5,38,48 }, 0.4, 0, 0);
	data_player.walk2.PushBack({ 317,4,44,50 }, 0.4, 0, 0);
	data_player.walk2.loop = true;

	//Sets the Jump Animation
	data_player.jump.PushBack({ 7,128,52,43 }, 0.6, 0, 0);
	data_player.jump.PushBack({ 81,114,47,57 }, 0.3, 0, 0);
	data_player.jump.PushBack({ 155,113,42,58 }, 0.3, 0, 0);
	data_player.jump.PushBack({ 221,118,52,53 }, 0.2, 0, 0);
	data_player.jump.PushBack({ 285,116,53,55 }, 0.15, 0, 0);
	data_player.jump.PushBack({ 348,119,56,52 }, 0.05, 0, 0);
	data_player.jump.loop = false;

	//Sets the Fall Animation
	data_entity.fall.PushBack({ 348,119,56,52 }, 0.05, 0, 0);
	data_entity.fall.PushBack({ 423,129,53,42 }, 0.5, 0, 0);
	data_entity.fall.loop = true;

	//Sets the Attack Animation
	data_player.idle_attack.PushBack({ 7,759,58,53 }, 0.1, 0, 0);
	data_player.idle_attack.PushBack({ 70,758,60,54 }, 0.1, 0, 0);
	data_player.idle_attack.PushBack({ 130,763,61,50 }, 0.1, 0, 0);
	data_player.idle_attack.PushBack({ 201,763,59,51 }, 0.1, 0, 0);
	data_player.idle_attack.PushBack({ 270,758,56,54 }, 0.1, 0, 0);
	data_player.idle_attack.PushBack({ 333,757,39,54 }, 0.1, 0, 0);
	data_player.idle_attack.PushBack({ 397,759,38,53 }, 0.1, 0, 0);
	data_player.idle_attack.loop = false;

	//Sets the Death Animation
	data_entity.death.PushBack({ 8,464,48,55 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 72,456,62,63 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 150,454,69,65 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 257,464,82,56 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 367,447,83,72 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 488,447,84,73 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 609,457,94,62 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 734,455,113,65 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 894,445,127,74 }, 0.2, 0, 0);
	data_entity.death.PushBack({ 1054,433,116,87 }, 0.2, 0, 0);
	data_entity.death.loop = false;

}

void j1Player::CheckState(float dt)
{
	
	if(godmode==false){
		
		
		//LOG("VEL: %i VELRUN: %f", data_entity.v.x, data_entity.velrun);

		//if "D" is pressed animation walk forward 
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT && data_player.canjump == true) {

			current_state = WALK;
			data_entity.position.x += (data_entity.v.x*dt * LIMIT_TIMER);
			data_entity.flip = false;
			LOG("%f", dt);

			//if  "SPACE" is pressed when "D" is pressed, the player jumps forward
			if (App->input->GetKey(SDL_SCANCODE_SPACE)==KEY_DOWN) {		

				data_player.jumpenergy = (data_player.jumpvel);

				current_state = JUMP_WALK;
				App->audio->PlayFx(App->scene->jump_FX);
				LOG("%f", dt);

				//decrease the jump counter
				data_player.jumpCounter--;

				//when the character do the double jump, reset jump animation
				if (data_player.jumpCounter == 0) {
					data_player.jump.Reset();
				}

			}

			//if  "LSHIFT" is pressed when "D" is pressed, the player runs forward
			if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {		
				data_entity.position.x += (data_player.velrun*dt * LIMIT_TIMER);

				current_state = RUN;
				
				data_entity.flip = false;

				//if "SPACE" is pressed when "LSHIFT" is pressed, and when "D" is pressed, the player jumps running forward
				if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {		
					
					data_player.jumpCounter = 2;					//reset counter. If we dont do it, character cannot do the double jump while running

					data_player.right = true;						//player can jump forward
					data_player.left = false;
					App->audio->PlayFx(App->scene->jump_FX);


					current_state = JUMP_RUN;
					LOG("%f", dt);

					//decrease the jump counter
					data_player.jumpCounter--;

					//when the character do the double jump, reset jump animation
					if (data_player.jumpCounter == 0) {
						data_player.jump.Reset();
					}
				}

			}
		}

		//if "A" is pressed animation walk backward actives flips to the Blit
		else if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT && data_player.canjump == true) {

			current_state = WALK;
			data_entity.position.x -= (data_entity.v.x*dt * LIMIT_TIMER);
			data_entity.flip = true;
			LOG("%f", dt);

			
			//if "SPACE" is pressed when "A" is pressed, the player jumps backward
			if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {		

				data_player.jumpenergy = (data_player.jumpvel);

				current_state = JUMP_WALK;
				LOG("%f", dt);

				
				data_player.jumpCounter--;
				App->audio->PlayFx(App->scene->jump_FX);

				if (data_player.jumpCounter == 0) {
					data_player.jump.Reset();
				}
			}

			//if  "LSHIFT" is pressed when "A" is pressed, the player runs backward
			if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) {
				
				current_state = RUN;
				data_entity.position.x -= (data_player.velrun * dt*LIMIT_TIMER);
			
				data_entity.flip = true;

				//if "SPACE" is pressed when "LSHIFT" is pressed, and when "A" is pressed, the player jumps running backward
				if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) {		 

					data_player.jumpCounter = 2;			//reset counter. If we dont do it, character cannot do the double jump while running
					data_player.left = true;				//player can jump backward
					data_player.right = false;

					current_state = JUMP_RUN;
					LOG("%f", dt);

					data_player.jumpCounter--;
					App->audio->PlayFx(App->scene->jump_FX);

					if (data_player.jumpCounter == 0) {
						data_player.jump.Reset();
					}
				}
			}
		}

		//if "SPACE" is pressed the player jumps
		else if ((App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN) && (data_player.jumpCounter!=0)) {		//counter needs to be !=0, if not cannot do double jump
			
		data_player.jumpenergy = (data_player.jumpvel);
				current_state = JUMP_UP;
				data_entity.flip = false;
				data_player.jumpCounter--;
				App->audio->PlayFx(App->scene->jump_FX);
				LOG("%f", dt);

				
				if(data_player.jumpCounter==0){
					data_player.jump.Reset();
				}

		}


		if ((App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN) && data_player.canjump == true || (data_player.TimeAttack == false)) {
						
			current_state = IDLE_ATTACK;
			
		}

		
		//If any key pressed animation idle
		else if(data_player.canjump==true && App->input->GetKey(SDL_SCANCODE_SPACE) == NULL && App->input->GetKey(SDL_SCANCODE_A) == NULL && App->input->GetKey(SDL_SCANCODE_D) == NULL && App->input->GetKey(SDL_SCANCODE_P)==NULL){
			
			current_state = IDLE;
			data_entity.flip = false;

		}
	}

}

void j1Player::State(float dt) {

	if (current_state == IDLE) {
		
		data_entity.current_animation = &data_entity.idle;				
		data_player.jump.Reset();
		data_entity.fall.Reset();
		data_player.idle_attack.Reset();
	}

	if (current_state == WALK) {

		data_entity.current_animation = &data_entity.walk;		
		data_player.jump.Reset();
		data_entity.fall.Reset();
		data_player.idle_attack.Reset();
	}
	if(current_state==JUMP_WALK){

		current_state = JUMP_UP;

	}

	if (current_state == JUMP_RUN) {
		
		//If left = true, jump running backward
		if (data_player.left == true) {
			
			data_entity.position.x -= (data_player.velrun*dt * LIMIT_TIMER);
		}

		//If right = true, jump running forward
		else if (data_player.right == true) {
			
			data_entity.position.x += (data_player.velrun*dt * LIMIT_TIMER);
		}

		data_player.canjump = false;
		data_player.injump = true;
		data_entity.current_animation = &data_player.jump;

		if ((data_player.jumpenergy) <= (data_entity.gravity)) {								//character will jump up until it do not accomplish this condition 
			
			data_player.jumpenergy += 1*dt * LIMIT_TIMER;													// jump up increments 0.5 each time
			data_entity.position.y += (data_player.jumpenergy);		// y position increments 0.5 each time

		}

		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		{
			data_entity.position.x -= data_entity.v.x*(dt * LIMIT_TIMER);
			data_entity.flip = true;
		}
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		{
			data_entity.position.x += data_entity.v.x*(dt * LIMIT_TIMER);
			data_entity.flip = false;
		}

	}
		
	if (current_state == RUN) {
		data_entity.current_animation = &data_player.walk2;
		data_player.jump.Reset();
		data_entity.fall.Reset();
		data_player.idle_attack.Reset();
	}

	if (current_state == JUMP_UP) {

		data_player.canjump = false;
		data_player.injump = true;
		data_entity.current_animation = &data_player.jump;

		if ((data_player.jumpenergy) <= (data_entity.gravity)) {									//character will jump up until it do not accomplish this condition 
			
			data_player.jumpenergy += 1*dt * LIMIT_TIMER;														// jump up increments 0.5 each time
			data_entity.position.y += (data_player.jumpenergy);		// y position increments 0.5 each time

		}

		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		{
			data_entity.position.x -= data_entity.v.x*(dt * LIMIT_TIMER);
			data_entity.flip = true;
		}
		
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		{
			data_entity.position.x += data_entity.v.x*(dt * LIMIT_TIMER);
			data_entity.flip = false;
		}
		
	}
	
	if (current_state == JUMP_FALL) {

		data_entity.current_animation = &data_entity.fall;
		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT)
		{
			data_entity.position.x -= data_entity.v.x*(dt * LIMIT_TIMER);
			data_entity.flip = true;
		}
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT)
		{
			data_entity.position.x += data_entity.v.x*(dt * LIMIT_TIMER);
			data_entity.flip = false;
			
		}
	}

	if (current_state == IDLE_ATTACK) {

		data_entity.current_animation = &data_player.idle_attack;
		//LOG("%i", data_entity.canjump);
		data_player.TimeAttack = true;
		if (data_entity.current_animation->AnimFinished() == true) {
			
			data_player.TimeAttack = false;
			
		}
		
	}

	if (current_state == DEATH) {	
		
		data_entity.die = true;	//Sets the die to true
		LOG("GLOBAL: %d", data_entity.globaltime);
		LOG("PRE: %d", data_entity.pretimer);
		if (data_entity.die == true) {

			if (App->scene->current_map == "Map.tmx") {	//If player is in map 1
				
				if (PreTime (20)) {	//Do a timer to stop the game during the Death animation
					

					
					//Sets the Position that player goes when he dies
					data_entity.position.x = 100;	//Set Player X	
					data_entity.position.y = 300;	//Set Player Y
					current_state = JUMP_FALL;	//Sets the Animation when he reapears
					data_entity.death.Reset();
				}

			}

			else {	//If player is not in map 1 is in map 2

				

				if (PreTime (20)) {	//Do a timer to stop the game during the Death Animation
					data_entity.current_animation = &data_entity.death;	//Current Animation is Death
					App->audio->PlayFx(App->scene->death_FX);	//Sets the Death Audio
					//Sets the Position that player goes when he dies
					data_entity.position.x = 55;	//Set Player X	
					data_entity.position.y = 10;	//Set Player Y
					current_state = JUMP_FALL;	//Sets the Animation when he reapears
					data_entity.death.Reset();

				}

			}

					
		}
		data_entity.die = false;

	}
	
}

void j1Player::OnCollision(Collider* c1, Collider* c2) {	//Check if the Player collides with something

	if(godmode==false){

		if (c1->type == ColliderType::COLLIDER_PLAYER && c2->type == ColliderType::COLLIDER_FLOOR) {	//If player collide with floor

			if (data_entity.preposition.y < c2->rect.y || data_entity.position.y == c2->rect.y - data_entity.entity_colliders->rect.h) {	//Checks that player collider from above

				data_entity.position.y = c2->rect.y - data_entity.entity_colliders->rect.h;
				data_entity.grounded = true;	//Sets that player is touching the floor
				data_player.canjump = true;		//Sets tha player can jump

				if (data_player.injump == true) {
					data_player.jump.Reset();
				}
				if (data_entity.grounded == true) {
					data_player.jumpCounter = 2;
				}

			}

			else if (data_entity.preposition.y > (c2->rect.y + c2->rect.h)) {	//Checks that player collider from below
			
				data_entity.position.y = c2->rect.y + c2->rect.h;
				current_state = JUMP_FALL;	//Sets the animation 

			}
		
			else if ((data_entity.position.x < c2->rect.x + c2->rect.w && data_entity.position.x > c2->rect.x) ||(data_entity.position.x + data_entity.entity_colliders->rect.w < c2->rect.x + c2->rect.w && data_entity.position.x + data_entity.entity_colliders->rect.w > c2->rect.x)) {	//Checks that player collider from sides
			
				if ((data_entity.position.x + data_entity.entity_colliders->rect.w) < (c2->rect.x + c2->rect.w)) { //Checks that player collides from left
				
					data_entity.position.x = c2->rect.x - data_entity.entity_colliders->rect.w;

				}
				else if (data_entity.position.x < (c2->rect.x + c2->rect.w)) {	//Checks that player collides from right
				
					data_entity.position.x = c2->rect.x + c2->rect.w;

				}
			}

		}

		if (c1->type == ColliderType::COLLIDER_PLAYER && c2->type == ColliderType::COLLIDER_PLATFORM) {		//Checks that player collides with platform

			if (data_entity.preposition.y < c2->rect.y || data_entity.position.y == c2->rect.y - data_entity.entity_colliders->rect.h) {	//Checks that player collider from above

				data_entity.position.y = c2->rect.y - data_entity.entity_colliders->rect.h;
				data_entity.grounded = true;	//Sets that player is touching the floor
				data_player.canjump = true;		//Sets tha player can jump

				if (data_entity.grounded == true) {
				
					data_player.jumpCounter = 2;

				}

			}
			else if ((data_entity.position.y >= data_entity.preposition.y) && (data_entity.preposition.y + data_entity.entity_colliders->rect.h) < c2->rect.y) {	//Checks that player collider from below
			
				data_entity.position.y = c2->rect.y - data_entity.entity_colliders->rect.h;

			}

		}

		if (c1->type == ColliderType::COLLIDER_PLAYER && c2->type == ColliderType::COLLIDER_DEAD) {		//Checks that player collides with something that he can die

			//PreTime = SDL_GetTicks();	//Sets the PreTime to death timer
		
			if (data_entity.preposition.y < c2->rect.y || data_entity.position.y == c2->rect.y - data_entity.entity_colliders->rect.h) {	//Checks that player collider from above
			
				data_entity.current_animation = &data_entity.death;	//Current Animation is Death
				App->audio->PlayFx(App->scene->death_FX);	//Sets the Death Audio
				data_entity.position.y = c2->rect.y - data_entity.entity_colliders->rect.h;
				current_state = DEATH;	//Sets player to Death state
			
				data_entity.grounded = true;	//Sets that player is touching the floor
				data_player.canjump = false;	//Sets tha player can jump
				//die = true;	//Sets die bool to true for timer start

			}

			else if (data_entity.preposition.y > (c2->rect.y + c2->rect.h)) {	//Checks that player collider from below
			
				data_entity.current_animation = &data_entity.death;	//Current Animation is Death
				App->audio->PlayFx(App->scene->death_FX);	//Sets the Death Audio
				//PreTime = SDL_GetTicks();	//Sets the PreTime to death timer

				//data_entity.position.y = c2->rect.y + c2->rect.h;
				current_state = DEATH;	//Sets player to Death state

				data_entity.grounded = true;	//Sets that player is touching the floor
				data_player.canjump = false;	//Sets tha player can jump
				//die = true;	//Sets die bool to true for timer start

			}

		}

		if (c1->type == ColliderType::COLLIDER_PLAYER && c2->type == ColliderType::COLLIDER_NEXT) {

			if (data_entity.preposition.y < c2->rect.y || data_entity.position.y == c2->rect.y - data_entity.entity_colliders->rect.h) {	//Checks that player collider from above	
				App->scene->SecondMap();	//Pass to next map
			}
		
			else if (data_entity.preposition.y > (c2->rect.y + c2->rect.h)) {	//Checks that player collider from below
				App->scene->SecondMap();	//Pass to next map
			}
		
			else if ((data_entity.position.x < c2->rect.x + c2->rect.w && data_entity.position.x > c2->rect.x) || (data_entity.position.x + data_entity.entity_colliders->rect.w < c2->rect.x + c2->rect.w && data_entity.position.x + data_entity.entity_colliders->rect.w > c2->rect.x)) {	//Checks that player collider from sides
			
				if ((data_entity.position.x + data_entity.entity_colliders->rect.w) < (c2->rect.x + c2->rect.w)) {		//Checks that player collides from left
					App->scene->SecondMap();	//Pass to next map
				}
				else if (data_entity.position.x < (c2->rect.x + c2->rect.w)) {	//Checks that player collides from right
					App->scene->SecondMap();	//Pass to next map
				}
			}
		}

	}
	else {

	}

}

void j1Player::Reset() {	//Reset All Player Animations

	data_entity.death.Reset();
	data_player.jump.Reset();
	data_entity.walk.Reset();
	data_player.walk2.Reset();
	data_entity.idle.Reset();
	data_entity.fall.Reset();
	data_player.running.Reset();
	data_entity.die = false;
}

bool j1Player::PreTime(float sec)
{
	bool ret = false;
	data_entity.pretimer++;
	if (data_entity.pretimer >= sec) { ret = true; data_entity.pretimer = 0; }
	return ret;
}