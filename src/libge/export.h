#pragma once

#ifdef LIBGE_EXPORTS
#define LIBGE_API __declspec(dllexport)
#else
#define LIBGE_API __declspec(dllimport)
#endif

#define LIBGE_NAMESPACE_BEGINE namespace LibGE{
#define LIBGE_NAMESPACE_END }

