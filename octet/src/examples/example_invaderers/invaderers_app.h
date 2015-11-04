////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
// invaderer example: simple game with sprites and sounds
//
// Level: 1
//
// Demonstrates:
//   Basic framework app
//   Shaders
//   Basic Matrices
//   Simple game mechanics
//   Texture loaded from GIF file
//   Audio
//

namespace octet {
	class sprite {
		// where is our sprite (overkill for a 2D game!)
		mat4t modelToWorld;

		// half the width of the sprite
		float halfWidth;

		// half the height of the sprite
		float halfHeight;

		// what texture is on our sprite
		int texture;

		// true if this sprite is enabled.
		bool enabled;
	public:
		sprite() {
			texture = 0;
			enabled = true;
		}

		void init(int _texture, float x, float y, float w, float h) {
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, 0);
			halfWidth = w * 0.5f;
			halfHeight = h * 0.5f;
			texture = _texture;
			enabled = true;
		}

		void set_texture(int _texture) {               //THIS IS FOR CHANGING THE TEXTURES
			texture = _texture;
		} //added to do: wanna change or delate

		void render(texture_shader &shader, mat4t &cameraToWorld) {
			// invisible sprite... used for gameplay.
			if (!texture) return;

			// build a projection matrix: model -> world -> camera -> projection
			// the projection space is the cube -1 <= x/w, y/w, z/w <= 1
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			// set up opengl to draw textured triangles using sampler 0 (GL_TEXTURE0)
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, texture);

			// use "old skool" rendering
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			shader.render(modelToProjection, 0);

			// this is an array of the positions of the corners of the sprite in 3D
			// a straight "float" here means this array is being generated here at runtime.
			float vertices[] = {
				-halfWidth, -halfHeight, 0,
				halfWidth, -halfHeight, 0,
				halfWidth,  halfHeight, 0,
				-halfWidth,  halfHeight, 0,
			};

			// attribute_pos (=0) is position of each corner
			// each corner has 3 floats (x, y, z)
			// there is no gap between the 3 floats and hence the stride is 3*sizeof(float)
			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
			glEnableVertexAttribArray(attribute_pos);

			// this is an array of the positions of the corners of the texture in 2D
			static const float uvs[] = {
				0,  0,
				1,  0,
				1,  1,
				0,  1,
			};

			// attribute_uv is position in the texture of each corner
			// each corner (vertex) has 2 floats (x, y)
			// there is no gap between the 2 floats and hence the stride is 2*sizeof(float)
			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
			glEnableVertexAttribArray(attribute_uv);

			// finally, draw the sprite (4 vertices)
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		void render(yuka_shader &shader, mat4t &cameraToWorld, int v_width, int v_height) {
			
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			shader.render(modelToProjection, vec2(v_width, v_height));

			float vertices[] = {
				-halfWidth, -halfHeight, 0,
				halfWidth, -halfHeight, 0,
				halfWidth,  halfHeight, 0,
				-halfWidth,  halfHeight, 0,
			};

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)vertices);
			glEnableVertexAttribArray(attribute_pos);

			static const float uvs[] = {
				0,  0,
				1,  0,
				1,  1,
				0,  1,
			};

			glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)uvs);
			glEnableVertexAttribArray(attribute_uv);

			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		// move the object
		void translate(float x, float y) {
			modelToWorld.translate(x, y, 0);
		}

		// position the object relative to another.
		void set_relative(sprite &rhs, float x, float y) {
			modelToWorld = rhs.modelToWorld;
			modelToWorld.translate(x, y, 0);
		}

		// return true if this sprite collides with another.
		// note the "const"s which say we do not modify either sprite
		bool collides_with(const sprite &rhs) const {
			float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];
			float dy = rhs.modelToWorld[3][1] - modelToWorld[3][1];

			// both distances have to be under the sum of the halfwidths
			// for a collision
			return
				(fabsf(dx) < halfWidth + rhs.halfWidth) &&
				(fabsf(dy) < halfHeight + rhs.halfHeight)
				;
		}

		bool is_above(const sprite &rhs, float margin) const {
			float dx = rhs.modelToWorld[3][0] - modelToWorld[3][0];

			return
				(fabsf(dx) < halfWidth + margin)
				;
		}

		bool &is_enabled() {
			return enabled;
		}
	};

	class invaderers_app : public octet::app {
		// Matrix to transform points in our camera space to the world.
		// This lets us move our camera
		mat4t cameraToWorld;

		// shader to draw a textured triangle
		texture_shader texture_shader_;
		yuka_shader yuka_shader_;

		enum {
			num_sound_sources = 12,
			num_bigstar = 10,
			num_middlestar = 15,
			num_smallstar = 20,
			// num_rows = 5,
			//num_cols = 10,
			num_missiles = 12,
			num_bombs = 2,
			num_borders = 4,
			// num_invaderers = num_rows * num_cols,
			num_explosionsAnim = 8,
			num_explosions = 8,


			// sprite definitions

			ship_sprite = 0,




			first_bigstar_sprite,
			last_bigstar_sprite = first_bigstar_sprite + num_bigstar - 1,

			first_middlestar_sprite,
			last_middlestar_sprite = first_middlestar_sprite + num_middlestar - 1,

			first_smallstar_sprite,
			last_smallstar_sprite = first_smallstar_sprite + num_smallstar - 1,

			//  first_invaderer_sprite,
			//  last_invaderer_sprite = first_invaderer_sprite + num_invaderers - 1,
			test_sprite,

			first_missile_sprite,
			last_missile_sprite = first_missile_sprite + num_missiles - 1,

			first_bomb_sprite,
			last_bomb_sprite = first_bomb_sprite + num_bombs - 1,

			first_border_sprite,
			last_border_sprite = first_border_sprite + num_borders - 1,

			black_sprite,

			first_explosion_sprite,
			last_explosion_sprite = first_explosion_sprite + num_explosionsAnim*num_explosions - 1,

			game_over_sprite,
			game_clear_sprite,
			title_sprite,
			complete_sprite,


			num_sprites,

		};

		

		// timers for missiles and bombs
		int missiles_disabled;
		int bombs_disabled;
		int explosion_change;
		int counter;
		int currentExplosion;
		int autoMove_abled;
		int startDisabled;
		int startTimeCatcher;
		// stores current level
		int stage;
		static const int MAX_NR_LVL = 3;

		//bool for triggers
		bool explosion_trigger;

		// accounting for bad guys
		int live_invaderers;
		int num_lives;

		// game state
		bool game_over;
		bool game_clear;
		bool game_title;
		bool game_play;
		bool game_complete;

		long int score;
	
		// speed of enemy
		float invader_velocity;

		// sounds
		ALuint whoosh;
		ALuint bang;
		ALuint biing;
		ALuint pon;
		ALuint begin;
		ALuint end;
		ALuint clapping;
		ALuint bgm;
		int bgmPlayer;
		unsigned cur_source;
		ALuint sources[num_sound_sources];

		// big array of sprites
		sprite sprites[num_sprites];
		sprite background_sprite;
		sprite heart[5];
		sprite killed_invaderer_sprite;
		dynarray<sprite> inv_sprites;

		// random number generator
		class random randomizer;

		// a texture for our text
		GLuint font_texture;

		// information for our text
		bitmap_font font;

	
		//invaderer positions
		struct inv_position {
			int x;
			int y;
		};

		inv_position current_invader_position;

		dynarray<inv_position> inv_formation;

		ALuint get_sound_source() { return sources[cur_source++ % num_sound_sources]; }

		//start to play when enter f5. Also make delay to start.
		void game_start() {
			// printf("game_start\n");

			if (game_play) {
				sprites[title_sprite].translate(20, 0);
				sprites[black_sprite].translate(20, 0);
				
				return;
			}
			else
				startDisabled++;
			if (startDisabled == 10) {
				startDisabled = 0;
			}
			if (is_key_down(key_f5)) {
				startTimeCatcher = startDisabled;
				if (game_play != true) {
					ALuint source = get_sound_source();
					alSourcei(source, AL_BUFFER, begin);
					alSourcePlay(source);
				}
			}
			if (startTimeCatcher - 1 == startDisabled) {
				if (game_play) {
					
				}

				if (game_over) {
					stage = 0;
					app_init();
				

					
				}
				if (game_clear) {
					float inherit_velocity = abs(invader_velocity);
					int inherit_score = score;
					int inherit_lives = num_lives;
					app_init();//?????????????????????????????????
					invader_velocity = inherit_velocity + 0.01f;
					score = inherit_score + stage * 500 * stage;
					num_lives = inherit_lives;
					printf("stage clear\n");

					ALuint source = get_sound_source();
					alSourcei(source, AL_BUFFER, begin);
					alSourcePlay(source);


					// load_next_level();

				}
				if (game_complete) {
					stage = 0;
					app_init();
					
				}
				game_title = false;
				game_clear = false;
				game_over = false;


				game_play = true;
				startTimeCatcher = 0;
				startDisabled = 0;
			}
		}

		// called when we hit an enemy
		void on_hit_invaderer() {
			printf("onHitInv\n");
			ALuint source = get_sound_source();
			alSourcei(source, AL_BUFFER, bang);
			alSourcePlay(source);

			live_invaderers--;
			score = score + 20;

			if (live_invaderers == 0) {

				if (stage >= MAX_NR_LVL) {
					sprites[complete_sprite].translate(-20, 0);
					ALuint source = get_sound_source();
					alSourcei(source, AL_BUFFER, end);
					alSourcePlay(source);
					game_play = false;
					game_complete = true;

					return;
				}

				game_play = false;
				game_clear = true;
				//sprites[game_clear_sprite].translate(-20, 0);

				printf("stage clear!!\n");

				ALuint source = get_sound_source();
				alSourcei(source, AL_BUFFER, clapping);
				alSourcePlay(source);
			}
		}

		// called when we are hit
		void on_hit_ship() {

			//printf("on hit ship\n");
			sprites[ship_sprite].set_relative(sprites[first_border_sprite], 0, 0);

			ALuint source = get_sound_source();
			alSourcei(source, AL_BUFFER, pon);
			alSourcePlay(source);

			if (--num_lives == 0) {
				game_play = false;
				game_over = true;

				sprites[game_over_sprite].translate(-20, 0);

				ALuint source = get_sound_source();
				alSourcei(source, AL_BUFFER, end);
				alSourcePlay(source);
			
			}

			autoMove_abled = 32;
		}

		void auto_move_ship() {
			if (autoMove_abled) {
				sprites[ship_sprite].translate(0, 0.05f);

				autoMove_abled--;
			}
		}
		//randomly places stars and move them
		void auto_move_stars() {

			float bigStar_speed = -0.01f;
			float middleStar_speed = -0.008f;
			float smallStar_speed = -0.005f;

			for (int i = 0; i != num_bigstar; i++) {
				sprites[first_bigstar_sprite + i].translate(0, bigStar_speed);
				if (sprites[first_bigstar_sprite + i].collides_with(sprites[first_border_sprite])) {
					//printf("collide\n");
					float randomBackx = randomizer.get(0, 60) / 10.0f;
					float randomBacky = randomizer.get(0, 60) / 10.0f + 6.0f;
					sprites[first_bigstar_sprite + i].set_relative(sprites[first_border_sprite], -3, 0);
					sprites[first_bigstar_sprite + i].translate(randomBackx, randomBacky);
				}
			}
			for (int i = 0; i != num_middlestar; i++) {
				sprites[first_middlestar_sprite + i].translate(0, middleStar_speed);
				if (sprites[first_middlestar_sprite + i].collides_with(sprites[first_border_sprite])) {
					//printf("collide\n");
					float randomBackx = randomizer.get(0, 60) / 10.0f;
					float randomBacky = randomizer.get(0, 60) / 10.0f + 6.0f;
					sprites[first_middlestar_sprite + i].set_relative(sprites[first_border_sprite], -3, 0);
					sprites[first_middlestar_sprite + i].translate(randomBackx, randomBacky);
				}
			}
			for (int i = 0; i != num_smallstar; i++) {
				sprites[first_smallstar_sprite + i].translate(0, smallStar_speed);
				if (sprites[first_smallstar_sprite + i].collides_with(sprites[first_border_sprite])) {
					// printf("collide\n");
					float randomBackx = randomizer.get(0, 60) / 10.0f;
					float randomBacky = randomizer.get(0, 60) / 10.0f + 6.0f;
					sprites[first_smallstar_sprite + i].set_relative(sprites[first_border_sprite], -3, 0);
					sprites[first_smallstar_sprite + i].translate(randomBackx, randomBacky);
				}
			}
		}

		// use the keyboard to move the ship
		void move_ship() {
			const float ship_speed = 0.07f;
			// left and right arrows
			if (is_key_down(key_left)) {
				sprites[ship_sprite].translate(-ship_speed, 0);
				if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 2])) {
					sprites[ship_sprite].translate(+ship_speed, 0);
				}
			}
			else if (is_key_down(key_right)) {
				sprites[ship_sprite].translate(+ship_speed, 0);
				if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 3])) {
					sprites[ship_sprite].translate(-ship_speed, 0);
				}
			}
			else if (is_key_down(key_up)) {
				sprites[ship_sprite].translate(0, +ship_speed);
				if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 1])) {
					sprites[ship_sprite].translate(0, -ship_speed);
				}
			}
			else if (is_key_down(key_down)) {
				sprites[ship_sprite].translate(0, -ship_speed);
				if (sprites[ship_sprite].collides_with(sprites[first_border_sprite + 0])) {
					sprites[ship_sprite].translate(0, +ship_speed);
				}
			}
			//ship dies if it collides enemy 
			for (int i = 0; i != inv_sprites.size(); i++) {
				sprite &invaderer = inv_sprites[i];
				sprite &ship = sprites[ship_sprite];
				if (invaderer.is_enabled() && ship.collides_with(invaderer)) {
					on_hit_ship();
				}
			}

		}

		// fire button (space)
		void fire_missiles() {
			if (is_key_down(' ')) {
				//printf("down\n");
				if (missiles_disabled) {
					--missiles_disabled;
				}
				else {
					//printf("fire\n");

					for (int i = 0; i != num_missiles; ++i) {
						if (!sprites[first_missile_sprite + i].is_enabled()) {
							sprites[first_missile_sprite + i].set_relative(sprites[ship_sprite], 0, 0.5f);
							sprites[first_missile_sprite + i].is_enabled() = true;
							missiles_disabled = 4;

							score++;
							ALuint source = get_sound_source();
							alSourcei(source, AL_BUFFER, whoosh);
							alSourcePlay(source);
							break;
						}
					}

				}
			}
		}


		// pick and invader and fire a bomb
		void fire_bombs() {
			if (bombs_disabled) {
				--bombs_disabled;
			}
			else {
				// find an invaderer
				sprite &ship = sprites[ship_sprite];
				for (int j = randomizer.get(0, inv_sprites.size()); j < inv_sprites.size(); ++j) {
					sprite &invaderer = inv_sprites[j];//inv_sprites.size() inv_sprites[j];
					if (invaderer.is_enabled() && invaderer.is_above(ship, 0.3f)) {
						// find a bomb
						for (int i = 0; i != num_bombs; ++i) {
							if (!sprites[first_bomb_sprite + i].is_enabled()) {
								sprites[first_bomb_sprite + i].set_relative(invaderer, 0, -0.25f);
								sprites[first_bomb_sprite + i].is_enabled() = true;
								bombs_disabled = 30;
								ALuint source = get_sound_source();
								alSourcei(source, AL_BUFFER, biing);
								alSourcePlay(source);
								return;
							}
						}
						return;
					}
				}
			}
		}

		// animate the missiles
		void move_missiles() {
			const float missile_speed = 0.3f;
			for (int i = 0; i != num_missiles; ++i) {
				sprite &missile = sprites[first_missile_sprite + i];
				if (missile.is_enabled()) {
					missile.translate(0, missile_speed);
					for (int j = 0; j != inv_sprites.size(); ++j) {
						sprite &invaderer = inv_sprites[j];
						sprite &ship = sprites[ship_sprite];

						if (invaderer.is_enabled() && missile.collides_with(invaderer)) {
							printf("collide\n");
							killed_invaderer_sprite = inv_sprites[j];
							invaderer.is_enabled() = false;
							invaderer.translate(20, 0);
							missile.is_enabled() = false;
							missile.translate(20, 0);
							explosion_trigger = true;
							on_hit_invaderer();

							//
							//??????????
							// find explosions
							for (int i = 0; i != num_explosions*num_explosionsAnim; i++) {
								sprites[first_explosion_sprite + i].is_enabled() = false;
							}

							if (currentExplosion >= num_explosions) {
								currentExplosion = 0;
							}

							for (int i = 0; i != num_explosionsAnim; i++) {
								sprites[first_explosion_sprite + num_explosionsAnim * currentExplosion + i].set_relative(killed_invaderer_sprite, 0, 0);
								sprites[first_explosion_sprite + num_explosionsAnim * currentExplosion + i].is_enabled() = true;
							}

							currentExplosion++;

							goto next_missile;
						}
					}
					if (missile.collides_with(sprites[first_border_sprite + 1])) {
						missile.is_enabled() = false;
						missile.translate(20, 0);
					}
				}
			next_missile:;
			}
		}

		// animate the bombs
		void move_bombs() {
			const float bomb_speed = 0.2f;
			for (int i = 0; i != num_bombs; ++i) {
				sprite &bomb = sprites[first_bomb_sprite + i];
				if (bomb.is_enabled()) {
					bomb.translate(0, -bomb_speed);
					if (bomb.collides_with(sprites[ship_sprite])) {
						bomb.is_enabled() = false;
						bomb.translate(20, 0);
						bombs_disabled = 50;
						on_hit_ship();
						goto next_bomb;
					}
					if (bomb.collides_with(sprites[first_border_sprite + 0])) {
						bomb.is_enabled() = false;
						bomb.translate(20, 0);
					}
				}
			next_bomb:;
			}
		}

		// move the array of enemies
		void move_invaders(float dx, float dy) {
			for (int j = 0; j != inv_sprites.size(); ++j) {
				sprite &invaderer = inv_sprites[j];
				if (invaderer.is_enabled()) {
					invaderer.translate(dx, dy);
				}
			}
		}
		//animate the explosion
		void anim_explosion() {
			if (explosion_change) {
				explosion_change--;
			}
			else
			{
				// printf("called");
				if (counter >= num_explosions) {
					counter = 0;
				}
				for (int i = 0; i != num_explosions; ++i) {
					sprite &explosion = sprites[first_explosion_sprite + i*num_explosionsAnim];

					sprites[first_explosion_sprite + i*num_explosionsAnim + counter].translate(20, 0);
				}
				counter++;
				explosion_change = 0;
			}
		}

		// check if any invaders hit the sides.
		bool invaders_collide(sprite &border) {
			for (int j = 0; j != inv_sprites.size(); ++j) {
				sprite &invaderer = inv_sprites[j];
				if (invaderer.is_enabled() && invaderer.collides_with(border)) {
					return true;
				}
			}
			return false;
		}


		void draw_text(texture_shader &shader, float x, float y, float scale, const char *text) {
			mat4t modelToWorld;
			modelToWorld.loadIdentity();
			modelToWorld.translate(x, y, 0);
			modelToWorld.scale(scale, scale, 1);
			mat4t modelToProjection = mat4t::build_projection_matrix(modelToWorld, cameraToWorld);

			/*mat4t tmp;
			glLoadIdentity();
			glTranslatef(x, y, 0);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);
			glScalef(scale, scale, 1);
			glGetFloatv(GL_MODELVIEW_MATRIX, (float*)&tmp);*/

			enum { max_quads = 32 };
			bitmap_font::vertex vertices[max_quads * 4];
			uint32_t indices[max_quads * 6];
			aabb bb(vec3(0, 0, 0), vec3(256, 256, 0));

			unsigned num_quads = font.build_mesh(bb, vertices, indices, max_quads, text, 0);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, font_texture);

			shader.render(modelToProjection, 0);

			glVertexAttribPointer(attribute_pos, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].x);
			glEnableVertexAttribArray(attribute_pos);
			glVertexAttribPointer(attribute_uv, 3, GL_FLOAT, GL_FALSE, sizeof(bitmap_font::vertex), (void*)&vertices[0].u);
			glEnableVertexAttribArray(attribute_uv);

			glDrawElements(GL_TRIANGLES, num_quads * 6, GL_UNSIGNED_INT, indices);
		}

	public:

		// this is called when we construct the class
		invaderers_app(int argc, char **argv) : app(argc, argv), font(512, 256, "assets/big.fnt") {
		}

		// this is called once OpenGL is initialized
		void app_init() {
			// set up the shader
			yuka_shader_.init();
			texture_shader_.init();
			printf("relieved\n");
			// set up the matrices with a camera 5 units from the origin
			cameraToWorld.loadIdentity();
			cameraToWorld.translate(0, 0, 3);

			font_texture = resource_dict::get_texture_handle(GL_RGBA, "assets/big_0.gif");

			GLuint title = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/title2.gif");
			sprites[title_sprite].init(title, 0, 0, 4, 1.5f);

			GLuint complete = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/complete.gif");
			sprites[complete_sprite].init(complete, 20, 0, 5, 2.5f);

			GLuint ship = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/ship.gif");
			sprites[ship_sprite].init(ship, 0, -1.75f, 0.25f, 0.25f);

			GLuint GameOver = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameOver.gif");
			sprites[game_over_sprite].init(GameOver, 20, 0, 3, 1);

			GLuint GameClear = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/GameClear.gif");
			sprites[game_clear_sprite].init(GameClear, 20, 0, 2.5f, 2.5f);
		
			if (stage <1 || stage > MAX_NR_LVL) {
				printf("stage = 0");
				stage = 0;
			}
			load_next_level();

			// set the border to white for clarity
			GLuint white = resource_dict::get_texture_handle(GL_RGB, "#ffffff");
			sprites[first_border_sprite + 0].init(white, 0, -3, 6, 0.2f);
			sprites[first_border_sprite + 1].init(white, 0, 3, 6, 0.2f);
			sprites[first_border_sprite + 2].init(white, -3, 0, 0.2f, 6);
			sprites[first_border_sprite + 3].init(white, 3, 0, 0.2f, 6);
			GLuint black = resource_dict::get_texture_handle(GL_RGB, "#000000");
			sprites[black_sprite].init(black, 0, 0, 6, 6);

			GLuint green = resource_dict::get_texture_handle(GL_RGB, "#ff0000");
			background_sprite.init(green, 0, 0, 6, 6);

			//		  sprites[]

			//  load_next_level();	//	To do: why: can't put here.   

			// use the missile texture
			GLuint missile = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/missile.gif");
			for (int i = 0; i != num_missiles; ++i) {
				// create missiles off-screen
				sprites[first_missile_sprite + i].init(missile, 20, 0, 0.0625f, 0.25f);
				sprites[first_missile_sprite + i].is_enabled() = false;
			}

			// use the bomb texture
			GLuint bomb = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/bomb.gif");
			for (int i = 0; i != num_bombs; ++i) {
				// create bombs off-screen
				sprites[first_bomb_sprite + i].init(bomb, 20, 0, 0.0625f, 0.25f);//20,0
				sprites[first_bomb_sprite + i].is_enabled() = false;
			}

			GLuint smallStar = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/star.gif");
			for (int i = 0; i != num_smallstar; ++i) {
				// create bombs off-screen
				float randomx = randomizer.get(0, 54) / 10.0f - 3.0f;
				float randomy = randomizer.get(0, 64) / 10.0f - 3.0f;
				//printf("random x = %f, random y = %f\n", randomx, randomy);
				sprites[first_smallstar_sprite + i].init(smallStar, randomx, randomy, 0.02f, 0.02f);//
				sprites[first_smallstar_sprite + i].is_enabled() = true;
			}

			GLuint middleStar = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/star.gif");
			for (int i = 0; i != num_middlestar; ++i) {
				float randomx = randomizer.get(0, 54) / 10.0f - 3.0f;
				float randomy = randomizer.get(0, 64) / 10.0f - 3.0f;
				sprites[first_middlestar_sprite + i].init(smallStar, randomx, randomy, 0.04f, 0.04f);//
				sprites[first_middlestar_sprite + i].is_enabled() = true;
			}

			GLuint bigStar = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/star.gif");
			for (int i = 0; i != num_bigstar; ++i) {
				float randomx = randomizer.get(0, 54) / 10.0f - 3.0f;
				float randomy = randomizer.get(0, 64) / 10.0f - 3.0f;
				sprites[first_bigstar_sprite + i].init(smallStar, randomx, randomy, 0.07f, 0.07f);//
				sprites[first_bigstar_sprite + i].is_enabled() = true;
			}




			// use the explosion textures
			GLuint explosion[num_explosionsAnim];
			//flag
			explosion[0] = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion/0.gif");
			explosion[1] = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion/1.gif");
			explosion[2] = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion/2.gif");
			explosion[3] = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion/3.gif");
			explosion[4] = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion/4.gif");
			explosion[5] = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion/5.gif");
			explosion[6] = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion/6.gif");
			explosion[7] = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/explosion/7.gif");
			for (int j = 0; j != num_explosions; ++j) {
				for (int i = 0; i < num_explosionsAnim; ++i) {
					// create explosions off-screen
					sprites[first_explosion_sprite + i + num_explosions*j].init(explosion[i], 20*i, 20*j, 0.25f, 0.25f);//0.2*i, 0.2*j, 0.25f,0.25f
					sprites[first_explosion_sprite + i + num_explosions*j].is_enabled() = false;
				}
			}

			// sounds
			whoosh = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/piyo.wav");
			bang = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/twiun.wav");
			biing = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/biing.wav");
			pon = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/pon.wav");
			begin = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/begin.wav");
			end = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/lose.wav");
			clapping = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/clapping.wav");
			bgm = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bgm.wav");
			cur_source = 0;
			alGenSources(num_sound_sources, sources);

			// sundry counters and game state.
			missiles_disabled = 0;
			bombs_disabled = 50;
			autoMove_abled = 0;
			explosion_change = 5;
			explosion_trigger = false;
			invader_velocity = 0.01f;
			live_invaderers = inv_sprites.size();
			num_lives = 3;
			game_over = false;
			game_clear = false;
			game_play = false;
			game_title = true;
			game_complete = false;
			score = 0;
			counter = 0;
			currentExplosion = 0;
			startTimeCatcher = 0;
			startDisabled = 0;
			bgmPlayer = false;

		}

		// called every frame to move things
		void simulate() {

			//when state is title
			game_start();

			if (game_over || game_clear || game_title) {
				return;
			}

			move_ship();

			auto_move_ship();

			auto_move_stars();

			fire_missiles();

			fire_bombs();

			move_missiles();

			move_bombs();

			move_invaders(invader_velocity, 0);

			anim_explosion();

			sprite &border = sprites[first_border_sprite + (invader_velocity < 0 ? 2 : 3)];
			if (invaders_collide(border)) {
				invader_velocity = -invader_velocity;
				move_invaders(invader_velocity, -0.1f);
			}
		}



		// this is called to draw the world
		void draw_world(int x, int y, int w, int h) {

			if (is_key_going_down(key_rmb)) {
				load_next_level();
			}

			simulate();

			// set a viewport - includes whole window area
			glViewport(x, y, w, h);

			// clear the background to black
			glClearColor(0, 0, 0, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			// don't allow Z buffer depth testing (closer objects are always drawn in front of far ones)
			glDisable(GL_DEPTH_TEST);

			// allow alpha blend (transparency when alpha channel is 0)
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

			// draw background
			background_sprite.render(yuka_shader_, cameraToWorld, w, h);

			for (int i = 0; i < inv_sprites.size(); ++i) {
				inv_sprites[i].render(texture_shader_, cameraToWorld);
			}

			// draw all the sprites
			for (int i = 0; i != num_sprites; ++i) {
				sprites[i].render(texture_shader_, cameraToWorld);
			}

			if (game_play || game_clear) {
				char score_text[32];
				sprintf(score_text, "lives: %d   score: %d", num_lives, score);
				draw_text(texture_shader_, -1.75f, 2, 1.0f / 256, score_text);
				char stage_text[32];
				sprintf(stage_text, "stage: %d", stage);
				draw_text(texture_shader_, 2.25f, 2, 1.0f / 256, stage_text);
			}
			if (game_title) {
				char title_text[32];
				sprintf(title_text, "press f5 to start", score, num_lives);
				draw_text(texture_shader_, 0, -2.3, 1.0f / 256, title_text);
			}
			if (game_clear) {
				char clear_message[40];
				sprintf(clear_message, "STAGE %d CLEAR !\nPress f5 to next", stage);
				draw_text(texture_shader_, 0, 0, 1.0f / 256, clear_message);
			}
			if (game_over) {
				char score_text[40];
				sprintf(score_text, "press f5 to try again", score, num_lives);
				draw_text(texture_shader_, 0, -2.3, 1.0f / 256, score_text);
			}
			if (game_complete) {
				char score_text[40];
				sprintf(score_text, "press f5 to try again");
				draw_text(texture_shader_, 0, -2.3, 1.0f / 256, score_text);
			}
			// move the listener with the camera
			vec4 &cpos = cameraToWorld.w();
			alListener3f(AL_POSITION, cpos.x(), cpos.y(), cpos.z());
		}

		void read_file() {
			printf("read_file start\n");
			std::string s1("inv_formation");
			std::string s2 = std::to_string(stage);
			std::string s3(".csv");
			//printf("%s%s%s\n",s1,s2,s3);
			//char s2 = static_cast<char>(stage);
			std::ifstream file(s1 + s2 + s3);//to do: how to change level
											 //		std::ifstream file("inv_formation"+ std::to_string(stage)+".csv");

			printf("read csv file\n");
			inv_formation.resize(0);

			char buffer[2048];
			int i = 0;
			while (!file.eof()) {
				file.getline(buffer, sizeof(buffer));
				//printf("while loop \n");
				char *b = buffer;
				for (int j = 0;; ++j) {
					char *e = b;
					while (*e != 0 && *e != ',') {
						++e;
					}

					if (std::atoi(b) == 1) {
						printf("found 1 \n");
						//inv_position p;
						current_invader_position.x = j;
						current_invader_position.y = i;
						inv_formation.push_back(current_invader_position);
					}
					else {
						printf("found 0 \n");
					}

					if (*e != ',') {
						break;
					}
					b = e + 1;
				}
				++i;
			}

		}

		void load_next_level() {
			stage++;
			if (stage > MAX_NR_LVL) {
				return;
				// TODO: show you win screen
			}

			read_file();
			inv_sprites.resize(0);

			GLuint invaderer = resource_dict::get_texture_handle(GL_RGBA, "assets/invaderers/invaderer.gif");
			for (int i = 0; i < inv_formation.size(); ++i) {
				//sprite inv;
				sprite inv;
				inv.init(invaderer, -1.5f + 0.66f*inv_formation[i].x, 2 - 0.5f*inv_formation[i].y, 0.25f, 0.25f);
				inv_sprites.push_back(inv);
			}
		}
	};
}
