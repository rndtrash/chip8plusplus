#include "chip8.hpp"

Chip8::Chip8()
{
	init();
}

Chip8::Chip8(bool debug)
{
	this->debug = debug;
	init();
}

Chip8::~Chip8()
{
}

void Chip8::init()
{
	m_mt = std::mt19937(std::random_device{}());
	m_dist = std::uniform_int_distribution<int>(0, 0xFF);

	m_memory = {};
	m_stack = {};
	m_v = {};
	m_screen = {};
	keys = {};

	reset();
}

void Chip8::load(std::vector<uint8_t> rom)
{
	if (rom.size() > (m_memory.size() - 512))
	{
		throw std::runtime_error(fmt::format(
			"ERROR: ROM is larger than memory ({} bytes > {} bytes).",
			rom.size(), m_memory.size()));
	}

	reset();
	for (int i = 0; i < rom.size(); i++)
	{
		m_memory[i + 512] = rom[i];
	}
}

void Chip8::reset()
{
	m_pc = 512;
	m_opcode = 0;
	m_sp = 0;
	m_i = 0;

	m_delayTimer = 0;
	m_soundTimer = 0;

	m_memory.fill(0);
	for (int i = 0; i < m_font.size(); i++)
	{
		m_memory[i] = m_font[i];
	}

	m_stack.push(0);
	m_v.fill(0);
	for (int y = 0; y < m_screen.size(); y++)
	{
		std::array<bool, 64> a = {};
		a.fill(false);
		m_screen[y] = a;
	}
	keys.fill(0);
}

void Chip8::tick()
{
	m_screenChanged = false;

	m_opcode = m_memory[m_pc] << 8 | m_memory[m_pc + 1];
	executeOpcode(m_opcode);

	if (m_pc < m_memory.size()) m_pc += 2;

	if (m_delayTimer > 0)
	{
		m_delayTimer -= 1;
	}

	if (m_soundTimer > 0)
	{
		m_soundTimer -= 1;

		if (!m_soundTimer)
		{
			if (debug)
				std::cout << "Beep!" << std::endl;
		}
	}
}

std::array<std::array<bool, 64>, 32> Chip8::getScreen()
{
	return m_screen;
}

bool Chip8::isScreenChanged()
{
	return m_screenChanged;
}

void Chip8::executeOpcode(uint16_t o)
{
	uint16_t hi = o & 0xf000;

	if (m_instructions.count(hi))
	{
		if (debug) std::cout << fmt::format("Opcode: 0x{:04X}", o) << std::endl;
		m_instructions[hi](o);
	}
	else
	{
		instruction_invalid(o);
	}
}

void Chip8::instruction_invalid(uint16_t args)
{
	throw std::runtime_error(fmt::format("ERROR: Invalid opcode {:x}.", args));
}

void Chip8::instruction_sys(uint16_t args)
{
	uint16_t addr = args & 0x0fff;

	switch (addr)
	{
		case 0xE0:
			instruction_cls(args);
			break;
		case 0xEE:
			instruction_ret(args);
			break;
		default:
			break;
	}
}

void Chip8::instruction_cls(uint16_t)
{
	if (debug) std::cout << "CLS" << std::endl;

	std::array<bool, 64> x = {};
	x.fill(false);
	m_screen.fill(x);
}

void Chip8::instruction_ret(uint16_t)
{
	if (debug) std::cout << "RET" << std::endl;

	m_pc = m_stack.top();
	m_stack.pop();
}

void Chip8::instruction_jump(uint16_t args)
{
	uint16_t addr = args & 0x0fff;
	if (debug) std::cout << fmt::format("Jump to 0x{:03X}", addr) << std::endl;

	m_pc = addr - 2;
}

void Chip8::instruction_call(uint16_t args)
{
	uint16_t addr = args & 0x0fff;
	if (debug) std::cout << fmt::format("Set PC to 0x{:03X}", addr) << std::endl;

	m_stack.push(m_pc);
	m_pc = addr - 2;
}

void Chip8::instruction_se(uint16_t args)
{
	uint16_t x = (args & 0x0f00) >> 8;
	uint16_t kk = args & 0x00ff;

	if (m_v[x] == kk)
	{
		if (debug) std::cout << fmt::format("m_v[{}] == 0x{:02X}, skipping...",
			x, kk) << std::endl;
		m_pc += 2;
	}
}

void Chip8::instruction_sne(uint16_t args)
{
	uint16_t x = (args & 0x0f00) >> 8;
	uint16_t kk = args & 0x00ff;

	if (m_v[x] != kk)
	{
		if (debug) std::cout << fmt::format("m_v[{}] != 0x{:02X}, skipping...",
			x, kk) << std::endl;
		m_pc += 2;
	}
}

void Chip8::instruction_ld(uint16_t args)
{
	uint16_t x = (args & 0x0f00) >> 8;
	uint16_t kk = args & 0x00ff;
	if (debug) std::cout << fmt::format("Set m_v[{}] to 0x{:02X}", x, kk) << std::endl;
	m_v[x] = kk;
}

void Chip8::instruction_8(uint16_t args)
{
	uint16_t i = args & 0x000f;

	switch (i)
	{
		case 0x0:
			instruction_ld_vx_vy(args);
			break;
		default:
			instruction_invalid(args);
			break;
	}
}

void Chip8::instruction_ld_vx_vy(uint16_t args)
{
	uint16_t x = (args & 0x0f00) >> 8;
	uint16_t y = (args & 0x00f0) >> 4;
	if (debug) std::cout << fmt::format("Set m_v[{}] to m_v[{}]", x, y) << std::endl;
	m_v[x] = m_v[y];
}

void Chip8::instruction_add(uint16_t args)
{
	uint16_t x = (args & 0x0f00) >> 8;
	uint16_t kk = args & 0x00ff;
	m_v[x] += kk;
	if (debug) std::cout << fmt::format("Add 0x{:02X} to m_v[{}] (now =0x{:02X})",
		kk, x, m_v[x]) << std::endl;
}

void Chip8::instruction_ld_i(uint16_t args)
{
	uint16_t nnn = args & 0x0fff;
	m_i = nnn;
	if (debug) std::cout << fmt::format("Set I to 0x{:03X}", nnn) << std::endl;
}

void Chip8::instruction_jump_b(uint16_t args)
{
	uint16_t addr = args & 0x0fff;
        if (debug) std::cout << fmt::format("Jump to 0x{:03X} + {}",
		addr, m_v[0]) << std::endl;

	m_pc = addr + m_v[0];
}

void Chip8::instruction_rnd(uint16_t args)
{
	uint16_t x = (args & 0x0f00) >> 8;
	uint16_t kk = args & 0x00ff;
	uint8_t r = m_dist(m_mt) & kk;
	if (debug) std::cout << fmt::format("Set m_v[{}] to 0x{:02X} (random)",
		x, r) << std::endl;

	m_v[x] = r;
}

void Chip8::instruction_drw(uint16_t args)
{
	uint16_t x = m_v[(args & 0x0f00) >> 8];
	uint16_t y = m_v[(args & 0x00f0) >> 4];
	uint16_t n = (args & 0x000f);
	bool prev = 0;

	x = wrap(x, m_screen[0].size() - 1);
	y = wrap(y, m_screen.size() - 1);

	uint16_t xx = x;
	for (uint16_t byte = m_i; byte < m_i + n; byte++)
	{
		for (uint8_t bit = 0; bit < 8; bit++)
		{
			if (!m_screenChanged)
				prev = m_screen[y][xx];

			m_screen[y][xx] ^= (m_memory[byte] & 0x80 >> bit);

			if (!m_screenChanged && m_screen[y][xx] != prev)
				m_screenChanged = true;
			xx = wrap(++xx, m_screen[y].size() - 1);
		}
		y = wrap(++y, m_screen.size());
		xx = x;
	}

	m_v[0xF] = m_screenChanged;
}

void Chip8::instruction_e(uint16_t args)
{
	uint16_t i = args & 0x00ff;

	switch (i)
	{
		case 0x9E:
			instruction_skp(args);
			break;
		case 0xA1:
			instruction_sknp(args);
			break;
		default:
			instruction_invalid(args);
			break;
	}
}

void Chip8::instruction_skp(uint16_t args)
{
	uint16_t x = (args & 0x0f00) >> 8;

	if (keys[m_v[x]])
		m_pc += 2;
}

void Chip8::instruction_sknp(uint16_t args)
{
	uint16_t x = (args & 0x0f00) >> 8;

	if (!keys[m_v[x]])
		m_pc += 2;
}

uint16_t Chip8::wrap(uint16_t x, uint16_t max)
{
	if (x > max)
	{
		return x % max;
	}
	else
	{
		return x;
	}
}
