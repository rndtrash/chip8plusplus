#ifndef _CHIP8_HPP
#define _CHIP8_HPP

#include <array>
#include <iostream>
#include <stdexcept>
#include <map>
#include <vector>
#include <stack>
#include <functional>
#include <random>
#include <fmt/format.h>

class Chip8
{
public:
	Chip8();
	Chip8(bool debug);
	~Chip8();

	void load(std::vector<uint8_t> rom);
	void reset();
	void tick();
	std::array<std::array<bool, 64>, 32> getScreen();
	bool isScreenChanged();

	std::array<bool, 16> keys;
	bool debug = false;
private:
	std::mt19937 m_mt;
	std::uniform_int_distribution<int> m_dist;

	bool m_screenChanged;

	uint16_t m_pc;
	uint16_t m_opcode;
	uint16_t m_sp;
	uint16_t m_i;

	uint8_t m_delayTimer;
	uint8_t m_soundTimer;

	std::array<uint8_t, 4096> m_memory;
	std::array<uint8_t, 16> m_v;
	std::array<std::array<bool, 64>, 32> m_screen;
	const std::array<uint8_t, 80> m_font =
	{
		0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
		0x20, 0x60, 0x20, 0x20, 0x70, // 1
		0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
		0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
		0x90, 0x90, 0xF0, 0x10, 0x10, // 4
		0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
		0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
		0xF0, 0x10, 0x20, 0x40, 0x40, // 7
		0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
		0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
		0xF0, 0x90, 0xF0, 0x90, 0x90, // A
		0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
		0xF0, 0x80, 0x80, 0x80, 0xF0, // C
		0xE0, 0x90, 0x90, 0x90, 0xE0, // D
		0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
		0xF0, 0x80, 0xF0, 0x80, 0x80  // F
	};

	std::stack<uint16_t> m_stack;

	std::map<uint16_t, std::function<void(uint16_t)>> m_instructions =
	{
		{ 0x0000, [this](uint16_t args) { instruction_sys(args); } },
		{ 0x1000, [this](uint16_t args) { instruction_jump(args); } },
		{ 0x2000, [this](uint16_t args) { instruction_call(args); } },
		{ 0x3000, [this](uint16_t args) { instruction_se(args); } },
		{ 0x4000, [this](uint16_t args) { instruction_sne(args); } },
		{ 0x6000, [this](uint16_t args) { instruction_ld(args); } },
		{ 0x7000, [this](uint16_t args) { instruction_add(args); } },
		{ 0x8000, [this](uint16_t args) { instruction_8(args); } },
		{ 0xA000, [this](uint16_t args) { instruction_ld_i(args); } },
		{ 0xB000, [this](uint16_t args) { instruction_jump_b(args); } },
		{ 0xC000, [this](uint16_t args) { instruction_rnd(args); } },
		{ 0xD000, [this](uint16_t args) { instruction_drw(args); } },
		{ 0xE000, [this](uint16_t args) { instruction_e(args); } }
	};

	void init();

	void executeOpcode(uint16_t o);

	void instruction_invalid(uint16_t args);

	void instruction_sys(uint16_t args); // 0nnn
	void instruction_cls(uint16_t); // 00E0
	void instruction_ret(uint16_t); // 00EE
	void instruction_jump(uint16_t args); // 1nnn
	void instruction_call(uint16_t args); // 2nnn
	void instruction_se(uint16_t args); // 3xkk
	void instruction_sne(uint16_t args); // 4xkk
	void instruction_ld(uint16_t args); // 6xkk
	void instruction_add(uint16_t args); // 7xkk
	void instruction_8(uint16_t args); // 8xy.
	void instruction_ld_vx_vy(uint16_t args); // 8xy0
	void instruction_ld_i(uint16_t args); // Annn
	void instruction_jump_b(uint16_t args); // Bnnn
	void instruction_rnd(uint16_t args); // Cxkk
	void instruction_drw(uint16_t args); // Dxyn
	void instruction_e(uint16_t args); // Ex..
	void instruction_skp(uint16_t args); // Ex9E
	void instruction_sknp(uint16_t args); // ExA1

	uint16_t wrap(uint16_t i, uint16_t max);
};

#endif
