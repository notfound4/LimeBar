#pragma once

namespace limebar
{
	class Module_wrapper;

	class Wrapper_Module
	{
	public:
		explicit Wrapper_Module(Module_wrapper *parent) : m_parent(parent) {}
		virtual ~Wrapper_Module() {}

		virtual void handle_output(char **args) = 0;

	protected:
		Module_wrapper *m_parent;
	};

	typedef Wrapper_Module* create_wrapper_module_t(Module_wrapper *);
	typedef void destroy_wrapper_module_t(Wrapper_Module*);

	struct wrapper_module_entry
	{
		Wrapper_Module *m_module;
		create_wrapper_module_t *m_creator;
		destroy_wrapper_module_t *m_destroyer;
		void *m_library;
	};
}