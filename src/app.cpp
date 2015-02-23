#include "app.hpp"

#include "geometry.hpp"

#include <iostream>
#include <stdexcept>


namespace blank {

Application::Application()
: init_sdl()
, init_img()
, init_gl()
, window()
, ctx(window.CreateContext())
, init_glew()
, program()
, move_velocity(0.003f)
, pitch_sensitivity(-0.0025f)
, yaw_sensitivity(-0.001f)
, blockType()
, cam()
, chunk()
, light_position(17.0f, 17.0f, 17.0f)
, light_color(1.0f, 1.0f, 1.0f)
, light_power(250.0f)
, m_handle(0)
, v_handle(0)
, mv_handle(0)
, mvp_handle(0)
, light_position_handle(0)
, light_color_handle(0)
, light_power_handle(0)
, running(false)
, front(false)
, back(false)
, left(false)
, right(false)
, up(false)
, down(false) {
	GLContext::EnableVSync();
	GLContext::EnableDepthTest();
	GLContext::EnableBackfaceCulling();
	program.LoadShader(
		GL_VERTEX_SHADER,
		"#version 330 core\n"
		"layout(location = 0) in vec3 vtx_position;\n"
		"layout(location = 1) in vec3 vtx_color;\n"
		"layout(location = 2) in vec3 vtx_normal;\n"
		"uniform mat4 M;\n"
		"uniform mat4 V;\n"
		"uniform mat4 MVP;\n"
		"uniform vec3 light_position;\n"
		"out vec3 frag_color;\n"
		"out vec3 vtx_world;\n"
		"out vec3 normal;\n"
		"out vec3 eye;\n"
		"out vec3 light_direction;\n"
		"void main() {\n"
			"vec4 v = vec4(vtx_position, 1);\n"
			"gl_Position = MVP * v;\n"
			"vtx_world = (M * v).xyz;\n"
			"vec3 vtx_camera = (V * M * v).xyz;\n"
			"eye = vec3(0, 0, 0) - vtx_camera;\n"
			"vec3 light_camera = (V * v).xyz;\n"
			"light_direction = light_position + eye;\n"
			"normal = (V * M * vec4(vtx_normal, 0)).xyz;\n"
			"frag_color = vtx_color;\n"
		"}\n"
	);
	program.LoadShader(
		GL_FRAGMENT_SHADER,
		"#version 330 core\n"
		"in vec3 frag_color;\n"
		"in vec3 vtx_world;\n"
		"in vec3 normal;\n"
		"in vec3 eye;\n"
		"in vec3 light_direction;\n"
		"uniform mat4 MV;\n"
		"uniform vec3 light_position;\n"
		"uniform vec3 light_color;\n"
		"uniform float light_power;\n"
		"out vec3 color;\n"
		"void main() {\n"
			"vec3 ambient = vec3(0.1, 0.1, 0.1) * frag_color;\n"
			"vec3 specular = vec3(0.3, 0.3, 0.3);\n"
			"float distance = length(light_position - vtx_world);\n"
			"vec3 n = normalize(normal);\n"
			"vec3 l = normalize(light_direction);\n"
			"float cos_theta = clamp(dot(n, l), 0, 1);\n"
			"vec3 E = normalize(eye);\n"
			"vec3 R = reflect(-l, n);\n"
			"float cos_alpha = clamp(dot(E, R), 0, 1);\n"
			"color = ambient"
				" + frag_color * light_color * light_power * cos_theta / (distance * distance)"
				" + specular * light_color * light_power * pow(cos_alpha, 5) / (distance * distance);\n"
		"}\n"
	);
	program.Link();
	if (!program.Linked()) {
		program.Log(std::cerr);
		throw std::runtime_error("link program");
	}

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	cam.Position(glm::vec3(0, 4, 4));

	blockType.Add(BlockType(true, glm::vec3(1, 1, 1)));
	blockType.Add(BlockType(true, glm::vec3(1, 0, 0)));
	blockType.Add(BlockType(true, glm::vec3(0, 1, 0)));
	blockType.Add(BlockType(true, glm::vec3(0, 0, 1)));

	chunk.BlockAt(glm::vec3(0, 0, 0)) = Block(blockType[4]);
	chunk.BlockAt(glm::vec3(0, 0, 1)) = Block(blockType[1]);
	chunk.BlockAt(glm::vec3(1, 0, 0)) = Block(blockType[2]);
	chunk.BlockAt(glm::vec3(1, 0, 1)) = Block(blockType[3]);
	chunk.BlockAt(glm::vec3(2, 0, 0)) = Block(blockType[4]);
	chunk.BlockAt(glm::vec3(2, 0, 1)) = Block(blockType[1]);
	chunk.BlockAt(glm::vec3(3, 0, 0)) = Block(blockType[2]);
	chunk.BlockAt(glm::vec3(3, 0, 1)) = Block(blockType[3]);
	chunk.BlockAt(glm::vec3(2, 0, 2)) = Block(blockType[4]);
	chunk.BlockAt(glm::vec3(2, 0, 3)) = Block(blockType[1]);
	chunk.BlockAt(glm::vec3(3, 0, 2)) = Block(blockType[2]);
	chunk.BlockAt(glm::vec3(3, 0, 3)) = Block(blockType[3]);
	chunk.BlockAt(glm::vec3(1, 1, 0)) = Block(blockType[1]);
	chunk.BlockAt(glm::vec3(1, 1, 1)) = Block(blockType[4]);
	chunk.BlockAt(glm::vec3(2, 1, 1)) = Block(blockType[3]);
	chunk.BlockAt(glm::vec3(2, 2, 1)) = Block(blockType[2]);
	chunk.Invalidate();

	m_handle = program.UniformLocation("M");
	v_handle = program.UniformLocation("V");
	mv_handle = program.UniformLocation("MV");
	mvp_handle = program.UniformLocation("MVP");
	light_position_handle = program.UniformLocation("light_position");
	light_color_handle = program.UniformLocation("light_color");
	light_power_handle = program.UniformLocation("light_power");

	glClearColor(0.0, 0.0, 0.0, 1.0);
}

Application::~Application() {

}


void Application::Run() {
	running = true;
	Uint32 last = SDL_GetTicks();
	window.GrabMouse();
	while (running) {
		Uint32 now = SDL_GetTicks();
		int delta = now - last;
		Loop(delta);
		last = now;
	}
}

void Application::Loop(int dt) {
	HandleEvents();
	Update(dt);
	Render();
}


void Application::HandleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				switch (event.key.keysym.sym) {
					case SDLK_w:
						front = event.key.state == SDL_PRESSED;
						break;
					case SDLK_s:
						back = event.key.state == SDL_PRESSED;
						break;
					case SDLK_a:
						left = event.key.state == SDL_PRESSED;
						break;
					case SDLK_d:
						right = event.key.state == SDL_PRESSED;
						break;
					case SDLK_q:
						up = event.key.state == SDL_PRESSED;
						break;
					case SDLK_e:
						down = event.key.state == SDL_PRESSED;
						break;
				}
				break;
			case SDL_MOUSEMOTION:
				cam.RotateYaw(event.motion.xrel * yaw_sensitivity);
				cam.RotatePitch(event.motion.yrel * pitch_sensitivity);
				break;
			case SDL_QUIT:
				running = false;
				break;
			case SDL_WINDOWEVENT:
				switch (event.window.event) {
					case SDL_WINDOWEVENT_RESIZED:
						cam.Viewport(event.window.data1, event.window.data2);
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
}

void Application::Update(int dt) {
	glm::vec3 vel;
	if (right && !left) {
		vel.x = move_velocity;
	} else if (left && !right) {
		vel.x = -move_velocity;
	}
	if (up && !down) {
		vel.y = move_velocity;
	} else if (down && !up) {
		vel.y = -move_velocity;
	}
	if (back && !front) {
		vel.z = move_velocity;
	} else if (front && !back) {
		vel.z = -move_velocity;
	}
	cam.OrientationVelocity(vel);

	cam.Update(dt);

	Ray aim = cam.Aim();
	int blkid;
	float dist;
	if (chunk.Intersection(aim, glm::mat4(1.0f), &blkid, &dist)) {
		glm::vec3 pos = Chunk::ToCoords(blkid);
		std::cout << "pointing at: <" << pos.x << ", " << pos.y << ", " << pos.z << ">, "
			"distance: " << dist << std::endl;
	} else {
		std::cout << "pointing at: nothing" << std::endl;
	}
}

void Application::Render() {
	GLContext::Clear();

	program.Use();

	glm::mat4 m(1.0f);
	glm::mat4 mv(cam.View() * m);
	glm::mat4 mvp(cam.MakeMVP(m));
	glUniformMatrix4fv(m_handle, 1, GL_FALSE, &m[0][0]);
	glUniformMatrix4fv(v_handle, 1, GL_FALSE, &cam.View()[0][0]);
	glUniformMatrix4fv(mv_handle, 1, GL_FALSE, &mv[0][0]);
	glUniformMatrix4fv(mvp_handle, 1, GL_FALSE, &mvp[0][0]);
	glUniform3f(light_position_handle, light_position.x, light_position.y, light_position.z);
	glUniform3f(light_color_handle, light_color.x, light_color.y, light_color.z);
	glUniform1f(light_power_handle, light_power);

	chunk.Draw();

	window.Flip();
}

}
