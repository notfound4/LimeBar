#pragma once

namespace limebar
{
	class Module
	{
	public:
		Module() {}
		virtual ~Module() {}

		virtual void init() {}
		virtual void cleanup() {}
	};

	typedef Module* create_module_t();
	typedef void destroy_module_t(Module*);

	struct module_entry
	{
		Module *m_module;
		create_module_t *m_creator;
		destroy_module_t *m_destroyer;
		void *m_library;
	};
}