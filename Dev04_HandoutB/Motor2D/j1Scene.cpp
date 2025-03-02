#include "p2Defs.h"
#include "p2Log.h"
#include "j1App.h"
#include "j1Input.h"
#include "j1Textures.h"
#include "j1Audio.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1Scene.h"
#include "j1Player.h"
#include "j1Collider.h"
#include "j1EntityManager.h"
#include "j1Entity.h"
#include "j1Pathfinding.h"
#include "j1Fonts.h"
#include "j1Player.h"
#include "j1FadeToBlack.h"
#include "j1Gui.h"

#include "UI_Button.h"
#include "UI_Slider.h"
#include "UI_Label.h"
#include "j1Image.h"

#include "j1MainMenu.h"
#include "p2SString.h"

#include "..//Brofiler/Brofiler.h"

j1Scene::j1Scene() : j1Module()
{
	name.create("scene");
	scene_change = false;
	is_changed = false;
	first_level = false;
	scene_change_timer = false;
}

// Destructor
j1Scene::~j1Scene()
{}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& config)
{
	LOG("Loading Scene");
	pugi::xml_node map;
	pugi::xml_document n;

	for (map= config.child("map"); map; map = map.next_sibling("map"))
	{
		p2SString* lvl = new p2SString();

		lvl->create(map.attribute("name").as_string());
		map_list.add(lvl->GetString());

	}

	return true;

}

// Called before the first frame

// Loads map and audio
bool j1Scene::Start()
{
	if (App->map->active==true) {

		LOG("LOADING MAP");
		if (App->main_menu->continue_lvl == true) {
			App->LoadGame();
			App->main_menu->continue_lvl = false;
		}

		current_map = map_list.start->data;
	
		App->map->Load(current_map.GetString());

		CreateEntities();

		//load audio from map 1
		if (current_map == "Map.tmx") {
			App->audio->PlayMusic("audio/music/map1_music.ogg");
			jump_FX= App->audio->LoadFx("audio/fx/Jump.wav");
			death_FX = App->audio->LoadFx("audio/fx/Death.wav");
			coin_FX = App->audio->LoadFx("audio/fx/Coin.wav");
			heart_FX = App->audio->LoadFx("audio/fx/Heart.wav");
			save_FX = App->audio->LoadFx("audio/fx/Save.wav");
		}

		//load audio from map 2
		else if(current_map=="map2.tmx") {
			App->audio->PlayMusic("audio/music/map2_music.ogg");
			jump_FX = App->audio->LoadFx("audio/fx/Jump.wav");
			death_FX = App->audio->LoadFx("audio/fx/Death.wav");
			coin_FX = App->audio->LoadFx("audio/fx/Coin.wav");
			heart_FX = App->audio->LoadFx("audio/fx/Heart.wav");
			save_FX = App->audio->LoadFx("audio/fx/Save.wav");
		}

		App->gui->CreateImage(10, 15, Image, { 451,75,57,55 }, NULL, this);
		App->gui->CreateImage(14, 100, Image, { 541,77,51,54 }, NULL, this);

		App->gui->CreateLabel(400, 15, Label, "Score: ", NULL, this, NULL);
		App->gui->CreateLabel(800, 15, Label, "Time: ", NULL, this, NULL);

		Player_time = (UI_Label*)App->gui->CreateLabel(900, 15, Label_timer, "", NULL, this, &App->entity->timer);
		Timer_t.Start();

	}

	return true;
}

// Called each loop iteration
bool j1Scene::PreUpdate(float dt)
{
	BROFILER_CATEGORY("PreUpdate scene", Profiler::Color::DarkSalmon);

	// debug pathfing ------------------
	//static iPoint origin;
	//static bool origin_selected = false;
	
	/*int x, y;
	App->input->GetMousePosition(x, y);
	iPoint p = App->render->ScreenToWorld(x, y);
	p = App->map->WorldToMap(p.x, p.y);

	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN)
	{
		
		if (origin_selected == true)
		{
			App->pathfinding->CreatePath(origin, p);
			origin_selected = false;
			LOG("%d,%d,%d,%d", origin.x, origin.y, p.x, p.y);
		}
		else
		{
			origin = p;
		..
			origin_selected = true;
		}
	}*/

	return true;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	BROFILER_CATEGORY("Update scene", Profiler::Color::Salmon);

	static char score_timer[6];
	timer_game = start_time + ((uint)Timer_t.ReadSec());
	sprintf(score_timer, "%02i:%02i", timer_game / 60, timer_game % 60);
	Player_time->SetLabelText(score_timer);

	//if coin is colliding with player, it adds 1 in coin collector
	if (App->entity->is_coin == true) {
		App->entity->coins++;
		App->entity->score += 2;
		coins_earned = true;
		App->entity->is_coin = false;
		App->entity->is_score = false;
	}

	
	//LOG("%i", App->entity->coins);

	//if heart is colliding with player, it adds 1 in live collector
	if (App->entity->is_live_plus == true) {
		App->entity->lives++;
		App->entity->score++;

		lives_earned = true;
		App->entity->is_live_plus = false;

		if (App->entity->lives >= 3) {
			App->entity->lives = 3;
			lives_earned = true;
		}
	}
	
	//if enemy is colliding with player, it removes 1 in live collector
	if (App->entity->is_live_minus == true) {

		App->entity->lives--;
		App->entity->is_live_minus = false;
		lives_earned = true;

		if (App->entity->lives == 0) {

			
			App->main_menu->cont = 0;

			App->main_menu->is_menu = true;
			App->scene->coins_earned = true;
			App->scene->lives_earned = true;

			App->entity->coins = 0;
			App->entity->lives = 3;
			App->entity->score = 0;

			App->entity->CleanEntity();
			App->collider->CleanUp();
			

			App->fade->FadeToBlack(App->scene, App->main_menu);

			App->scene->is_pause = false;

		}
	}
	
	//Player_lives->SetTextFromNum(App->entity->lives);

	//LOG("%i", App->entity->lives);

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN) {

		cont2 = 0;
		is_pause = !is_pause;
		App->gui->Delete_Element(window);
		App->gui->Delete_Element(title_win);
		App->gui->Delete_Element(title);
		App->gui->Delete_Element(slider_volume);
		App->gui->Delete_Element(slider_left_volume);
		App->gui->Delete_Element(slider_right_volume);
		App->gui->Delete_Element(slider_fx);
		App->gui->Delete_Element(slider_left_fx);
		App->gui->Delete_Element(slider_right_fx);
		App->gui->Delete_Element(fx_volume);
		App->gui->Delete_Element(music_volume);
	
		App->gui->Delete_Element(close);
		App->gui->Delete_Element(close_game);
		App->gui->Delete_Element(menu);
	}

	if (App->input->GetKey(SDL_SCANCODE_GRAVE) == KEY_DOWN) {

		cont3 = 0;
		is_console = !is_console;
		App->gui->Delete_Element(win_console);
		App->gui->Delete_Element(win_tex_console);
		
	}

	if (is_console == false) {
		cont3 = 0;
		App->gui->Delete_Element(win_console);
		App->gui->Delete_Element(win_tex_console);
	}
	if (is_pause == false) {

		cont2 = 0;
		App->gui->Delete_Element(window);
		App->gui->Delete_Element(title_win);
		App->gui->Delete_Element(title);
		App->gui->Delete_Element(slider_fx);
		App->gui->Delete_Element(slider_left_fx);
		App->gui->Delete_Element(slider_right_fx);
		App->gui->Delete_Element(fx_volume);
		App->gui->Delete_Element(slider_volume);
		App->gui->Delete_Element(slider_left_volume);
		App->gui->Delete_Element(slider_right_volume);
		App->gui->Delete_Element(music_volume);
		App->gui->Delete_Element(close);
		App->gui->Delete_Element(close_game);
		App->gui->Delete_Element(menu);

	}

	int w, h;
	uchar* data = NULL;
	if (App->map->CreateWalkabilityMap(w, h, &data))
		App->pathfinding->SetMap(w, h, data);
	RELEASE_ARRAY(data);

	//save player position in every map
	if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN) {

		App->SaveGame();
	}

	//load player position in every map
	if (App->input->GetKey(SDL_SCANCODE_F6) == KEY_DOWN) {
		App->LoadGame();
	}

	if (App->input->GetKey(SDL_SCANCODE_KP_PLUS) == KEY_DOWN) {
		App->audio->Change_Volume(0.1, 1);
	}
	else if (App->input->GetKey(SDL_SCANCODE_KP_MINUS) == KEY_DOWN) {
		App->audio->Change_Volume(0.1, 0);
	}

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN) {

		first_level = true;
		FirstLevel();

	}

	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN) {

		SecondLevel();

	}

	if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN) {

		RestartCurrentLevel();

	}
	
	/*// Debug pathfinding ------------------------------
	int x = 0, y = 0;
	App->input->GetMousePosition(x, y);
	iPoint p = App->render->ScreenToWorld(x, y);
	p = App->map->WorldToMap(p.x, p.y);
	p = App->map->MapToWorld(p.x, p.y);

	App->render->Blit(debug_tex, p.x, p.y);

	const p2DynArray<iPoint>* path = App->pathfinding->GetLastPath();
	//LOG("%d", path->Count());

	for (uint i = 0; i < path->Count(); ++i)
	{
		iPoint pos = App->map->MapToWorld(path->At(i)->x, path->At(i)->y);
		App->render->Blit(debug_tex, pos.x, pos.y);
	}*/
	
	App->map->Draw();

	return true;
}

// Called each loop iteration
bool j1Scene::PostUpdate(float dt)
{
	BROFILER_CATEGORY("PreUpdate scene", Profiler::Color::LightSalmon);

	bool ret = true;

	if (lives_earned == true) {

		int i = App->entity->lives;
		LOG("%i", i);
		App->gui->Delete_Element(Player_lives);
		Player_lives= (UI_Label*)App->gui->CreateLabel(85, 30, Label, cad[i], NULL, this, NULL);

		int m = App->entity->score;
		LOG("%i", m);
		App->gui->Delete_Element(Player_score);
		Player_score = (UI_Label*)App->gui->CreateLabel(510, 15, Label, score_cad[m], NULL, this, NULL);

		lives_earned = false;

	}

	if (coins_earned == true) {

		int i = App->entity->coins;
		LOG("%i", i);
		App->gui->Delete_Element(Player_coins);
		Player_coins = (UI_Label*)App->gui->CreateLabel(85, 115, Label, cad[i], NULL, this, NULL);

		int j = App->entity->score;
		LOG("%i", j);
		App->gui->Delete_Element(Player_score);
		Player_score = (UI_Label*)App->gui->CreateLabel(510, 15, Label, score_cad[j], NULL, this, NULL);

		coins_earned = false;

	}

	/*if (App->entity->is_timer == true) {

		if (Timer_t.Read() > 1000){
			Timer_t.Start();
		}
		
		App->entity->is_timer = false;
		time_start = true;
	}
	
	if (time_start == true) {

		App->entity->seconds = Timer_t.ReadSec();
		int a = App->entity->seconds;
		App->gui->Delete_Element(Player_time);
		Player_time = (UI_Label*)App->gui->CreateLabel(610, 15, Label, cad[a], NULL, this);

		if (App->entity->seconds == 60)
		{
			App->entity->seconds = 0;
			App->entity->minutes ++;
		}

		if (App->entity->seconds < 10)
		{

			cad[2] = cad[1];
			cad[1] = cad[0];
			cad[0] = "0";
					   			
			int b = App->entity->minutes;
			App->gui->Delete_Element(Player_time2);
			Player_time2 = (UI_Label*)App->gui->CreateLabel(603, 15, Label, cad2[b], NULL, this);
		}
		
	}*/
	
	if (cont==0){
		
		App->scene->jump_FX = App->audio->LoadFx("audio/fx/Jump.wav");
		App->scene->death_FX = App->audio->LoadFx("audio/fx/Death.wav");
		coin_FX = App->audio->LoadFx("audio/fx/Coin.wav");
		heart_FX = App->audio->LoadFx("audio/fx/Heart.wav");
		save_FX = App->audio->LoadFx("audio/fx/Save.wav");

		App->entity->Enable();
		App->audio->Enable();
		App->collider->Enable();
		
		
		Player_time = (UI_Label*)App->gui->CreateLabel(900, 15, Label_timer, "", NULL, this, &App->entity->timer);

		App->gui->CreateLabel(400, 15, Label, "Score: ", NULL, this, NULL);
		App->gui->CreateLabel(800, 15, Label, "Time: ", NULL, this, NULL);
	
		
		cont++;

	}

	if(ret_s==false){
		return false;
	}

	if (is_pause) {

		if (cont2 == 0) {
			
			App->gui->Start();
			
			CreateSettings();
			
			cont2++;

		}
		
	}

	if (is_console) {

		if (cont3 == 0) {

			CreateConsole();

			cont3++;

		}
	}

	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	LOG("Freeing scene");
	return true;
}

//load function
bool j1Scene::Load(pugi::xml_node& data)
{
	LOG("Loading Scene state");
	current_map = data.child("scene").attribute("name").as_string();					//check which map have to be loaded and load it
	
	if (current_map == "Map.tmx") {
		
		first_level = true;
		FirstLevel();
	}
	
	else if (current_map == "map2.tmx") {
		
		SecondLevel();

	}

	/*App->map->CleanUp();
	App->collider->CleanUp();   //cleans the colliders and map and entities
	App->entity->CleanEntity();
	
	App->map->Load(current_map.GetString());
	App->collider->Start();
	CreateEntities();*/
	return true;
}

bool j1Scene::Save(pugi::xml_node& data) const
{
	LOG("Saving Scene state");
	pugi::xml_node scene = data.append_child("scene");
	scene.append_attribute("name") = current_map.GetString();

	return true;
}

void j1Scene::SecondMap() {
	
	if (scene_change == true && is_changed == false) {
		FirstLevel();
		is_changed = true;
	}
	if (scene_change == false && is_changed == false) {
		SecondLevel();
		is_changed = true;
	}
	is_changed = false;

}

void j1Scene::RestartCurrentLevel() {

	if (current_map == "Map.tmx") {
		
		App->map->CleanUp();
		App->entity->CleanEntity();
		Map1Entities();
		
		if (current_map == "Map.tmx") {
			App->map->Load("Map.tmx");
		}
		else if (current_map == "map2.tmx") {
			App->map->Load("map2.tmx");
		}
		App->collider->Start();
		App->entity->coins = 0;
		App->entity->lives = 3;
	}

	else if (current_map == "map2.tmx") {
		
	
		App->map->CleanUp();
		App->entity->CleanEntity();
		Map2Entities();

		if (current_map == "Map.tmx") {
			App->map->Load("Map.tmx");
		}
		else if (current_map == "map2.tmx") {
			App->map->Load("map2.tmx");
		}
		App->collider->Start();
		App->entity->coins = 0;
		App->entity->lives = 3;
	}
	
}

void j1Scene::FirstLevel() {

	if (first_level == true) {

		App->map->CleanUp();
		current_map.create("Map.tmx");
		App->map->Load(current_map.GetString());
		App->collider->Start();
		App->entity->CleanEntity();
		CreateEntities();

		App->entity->coins = 0;
		App->entity->lives = 3;

		scene_change = false;
		first_level = false;

		int w, h;
		uchar* data = NULL;
		if (App->map->CreateWalkabilityMap(w, h, &data))
			App->pathfinding->SetMap(w, h, data);
		RELEASE_ARRAY(data);

		if (current_map == "Map.tmx") {
			App->audio->PlayMusic("audio/music/map1_music.ogg");
			jump_FX = App->audio->LoadFx("audio/fx/Jump.wav");
			death_FX = App->audio->LoadFx("audio/fx/Death.wav");
			coin_FX = App->audio->LoadFx("audio/fx/Coin.wav");
			heart_FX = App->audio->LoadFx("audio/fx/Heart.wav");
			save_FX = App->audio->LoadFx("audio/fx/Save.wav");
		}

		cont = 0;

	}

	else if (first_level == false) {

		if (scene_change == false) {

			scene_change_timer = SDL_GetTicks();
			App->fade->FadeInScene(2);
			scene_change = true;
			cont = 0;
		}

		if (SDL_GetTicks() - scene_change_timer > 1040) {

			App->map->CleanUp();
			current_map.create("Map.tmx");
			App->map->Load(current_map.GetString());
			App->entity->CleanEntity();
			App->collider->Start();
			CreateEntities();

			App->entity->coins = 0;
			App->entity->lives = 3;
			scene_change = false;
			scene_change_timer = false;
			cont = 0;
		}

		if (current_map == "Map.tmx") {
			App->audio->PlayMusic("audio/music/map1_music.ogg");
			jump_FX = App->audio->LoadFx("audio/fx/Jump.wav");
			death_FX = App->audio->LoadFx("audio/fx/Death.wav");
			coin_FX = App->audio->LoadFx("audio/fx/Coin.wav");
			heart_FX = App->audio->LoadFx("audio/fx/Heart.wav");
			save_FX = App->audio->LoadFx("audio/fx/Save.wav");
		}

		int w, h;
		uchar* data = NULL;
		if (App->map->CreateWalkabilityMap(w, h, &data))
			App->pathfinding->SetMap(w, h, data);
		RELEASE_ARRAY(data);
		cont = 0;
	}
}

void j1Scene::SecondLevel() {

	
	App->map->CleanUp();
	current_map.create("map2.tmx");
	App->map->Load(current_map.GetString());
	App->entity->CleanEntity();
	App->collider->Start();
	CreateEntities();

	//load audio from map 2
	if (current_map == "map2.tmx") {
		App->audio->PlayMusic("audio/music/map2_music.ogg");
		jump_FX = App->audio->LoadFx("audio/fx/Jump.wav");
		death_FX = App->audio->LoadFx("audio/fx/Death.wav");
		coin_FX = App->audio->LoadFx("audio/fx/Coin.wav");
		heart_FX = App->audio->LoadFx("audio/fx/Heart.wav");
		save_FX = App->audio->LoadFx("audio/fx/Save.wav");
	}

	App->entity->coins = 0;
	App->entity->lives = 3;
	scene_change = true;

	int w, h;
	uchar* data = NULL;
	if (App->map->CreateWalkabilityMap(w, h, &data))
		App->pathfinding->SetMap(w, h, data);
	RELEASE_ARRAY(data);
	cont = 0;
}

void j1Scene::CreateEntities() {

	if (current_map == "Map.tmx") {

		App->entity->DrawEntity(100, 500, j1Entity::entity_type::PLAYER);
		/*App->entity->DrawEntity(2550, 200, j1Entity::entity_type::GOLEM_GRASS_ENEMY);
		App->entity->DrawEntity(5250, 400, j1Entity::entity_type::GOLEM_GRASS_ENEMY);
		App->entity->DrawEntity(4000, 200, j1Entity::entity_type::BAT_ENEMY);
		App->entity->DrawEntity(700, 200, j1Entity::entity_type::BAT_ENEMY);*/
		App->entity->DrawEntity(4110, 256, j1Entity::entity_type::HEART);
		App->entity->DrawEntity(4192, 1088, j1Entity::entity_type::HEART);
		App->entity->DrawEntity(1472, 256, j1Entity::entity_type::COIN);
		App->entity->DrawEntity(2336, 640, j1Entity::entity_type::COIN);
		App->entity->DrawEntity(5504, 1088, j1Entity::entity_type::COIN);
		App->entity->DrawEntity(4800, 1088, j1Entity::entity_type::COIN);

	}

	else if (current_map == "map2.tmx") {

		App->entity->DrawEntity(55, 100, j1Entity::entity_type::PLAYER);
		/*App->entity->DrawEntity(1500, 500, j1Entity::entity_type::GOLEM_ROCK_ENEMY);
		App->entity->DrawEntity(500, 100, j1Entity::entity_type::GOLEM_ROCK_ENEMY);
		App->entity->DrawEntity(700, 100, j1Entity::entity_type::BAT_ENEMY);
		App->entity->DrawEntity(2000, 1000, j1Entity::entity_type::BAT_ENEMY);*/
		App->entity->DrawEntity(704, 384, j1Entity::entity_type::COIN);
		App->entity->DrawEntity(1376, 1984, j1Entity::entity_type::COIN);
		App->entity->DrawEntity(2752, 1152, j1Entity::entity_type::COIN);
		App->entity->DrawEntity(5700, 1440, j1Entity::entity_type::COIN);
		App->entity->DrawEntity(5900, 1376, j1Entity::entity_type::COIN);

		App->entity->DrawEntity(4544, 2280, j1Entity::entity_type::HEART);
		App->entity->DrawEntity(3360, 1100, j1Entity::entity_type::HEART);
		
	}

}

void j1Scene::Map1Entities() {

	App->entity->CleanEntity();
	App->entity->DrawEntity(100, 500, j1Entity::entity_type::PLAYER);
	/*App->entity->DrawEntity(2550, 200, j1Entity::entity_type::GOLEM_GRASS_ENEMY);
	App->entity->DrawEntity(5250, 400, j1Entity::entity_type::GOLEM_GRASS_ENEMY);
	App->entity->DrawEntity(4000, 200, j1Entity::entity_type::BAT_ENEMY);
	App->entity->DrawEntity(700, 200, j1Entity::entity_type::BAT_ENEMY);*/
	App->entity->DrawEntity(4110, 256, j1Entity::entity_type::HEART);
	App->entity->DrawEntity(4192, 1088, j1Entity::entity_type::HEART);
	App->entity->DrawEntity(1472, 256, j1Entity::entity_type::COIN);
	App->entity->DrawEntity(2336, 640, j1Entity::entity_type::COIN);
	App->entity->DrawEntity(5504, 1088, j1Entity::entity_type::COIN);
	App->entity->DrawEntity(4800, 1088, j1Entity::entity_type::COIN);

	App->collider->Start();
}

void j1Scene::Map2Entities() {
	

	App->entity->CleanEntity();
	App->entity->DrawEntity(55, 100, j1Entity::entity_type::PLAYER);
	/*App->entity->DrawEntity(1500, 500, j1Entity::entity_type::GOLEM_ROCK_ENEMY);
	App->entity->DrawEntity(500, 100, j1Entity::entity_type::GOLEM_ROCK_ENEMY);
	App->entity->DrawEntity(700, 100, j1Entity::entity_type::BAT_ENEMY);
	App->entity->DrawEntity(2000, 1000, j1Entity::entity_type::BAT_ENEMY);*/
	App->entity->DrawEntity(704, 384, j1Entity::entity_type::COIN);
	App->entity->DrawEntity(1376, 1984, j1Entity::entity_type::COIN);
	App->entity->DrawEntity(2752, 1152, j1Entity::entity_type::COIN);
	App->entity->DrawEntity(5700, 1440, j1Entity::entity_type::COIN);
	App->entity->DrawEntity(5900, 1376, j1Entity::entity_type::COIN);

	App->entity->DrawEntity(4544, 2280, j1Entity::entity_type::HEART);
	App->entity->DrawEntity(3360, 1100, j1Entity::entity_type::HEART);
	
	App->collider->Start();

}

void j1Scene::CreateSettings() {

	LOG("SETTINGS CREATED");

	window = (j1Image*)App->gui->CreateImage(325, 280, Image, { 0, 351, 384, 268 }, NULL, this);
	title_win = (j1Image*)App->gui->CreateImage(410, 205, Image, { 2, 278, 202, 58 }, NULL, this);

	close = (UI_Button*)App->gui->CreateButton(645, 296, Button_close, { 0,0,44,44 }, { 69,0,44,44 }, { 141, 0, 44, 44 }, NULL, this);
	
	slider_left_volume = (UI_Button*)App->gui->CreateButton(360, 335, Button_slider_music_left, { 0,165,28,35 }, { 0,165,28,35 }, { 0,165,28,35 }, NULL, this);
	slider_right_volume = (UI_Button*)App->gui->CreateButton(630, 335, Button_slider_music_right, { 262,165,30,36 }, { 262,165,30,36 }, { 262,165,30,36 }, NULL, this);
	slider_volume = (UI_Slider*)App->gui->CreateSlider(400, 340, Slider_music, { 38,169,214,24 }, { 125,221,34,36 }, 200, NULL, this);

	slider_left_fx = (UI_Button*)App->gui->CreateButton(360, 435, Button_slider_fx_left, { 0,165,28,35 }, { 0,165,28,35 }, { 0,165,28,35 }, NULL, this);
	slider_right_fx = (UI_Button*)App->gui->CreateButton(630, 435, Button_slider_fx_right, { 262,165,30,36 }, { 262,165,30,36 }, { 262,165,30,36 }, NULL, this);
	slider_fx = (UI_Slider*)App->gui->CreateSlider(400, 440, Slider_fx, { 38,169,214,24 }, { 125,221,34,36 }, 200, NULL, this);

	music_volume = (UI_Label*)App->gui->CreateLabel(402, 300, Label, "Music Volume", NULL, this, NULL);
	fx_volume = (UI_Label*)App->gui->CreateLabel(425, 400, Label, "FX Volume", NULL, this, NULL);
	title = (UI_Label*)App->gui->CreateLabel(415, 220, Label, "Pause Menu", NULL, this, NULL);

	close_game = (UI_Button*)App->gui->CreateButton(485, 490, Button_close_game, { 475,286,43,43 }, { 599,286,43,43 }, { 718, 286,43,43 }, NULL, this);
	menu = (UI_Button*)App->gui->CreateButton(350, 480, Button_menu, { 472,337,51,51 }, { 596,337,51,51 }, { 714, 337,51,51 }, NULL, this);

}

void j1Scene::CreateConsole() {

	LOG("SETTINGS CREATED");

	win_console = (j1Image*)App->gui->CreateImage(0, 0, Image, { 0, 675, 679, 223 }, NULL, this);
	win_tex_console = (j1Image*)App->gui->CreateImage(0, 240, Image, { 0, 624, 682, 50 }, NULL, this);

}