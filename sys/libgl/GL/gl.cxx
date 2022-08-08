#include <stddef.h>
#include <limits.h>
#include <stdlib.h>
#include "GL/gl.h"

#define GL_MAX_LISTS 16

#define GL_UNIMPLEMENTED(...) \
	printf("%s: not implemented!\n", __FUNCTION__); \
	__builtin_unreachable();

enum gl_command_type {
	GL_COMMAND_COLOR,
	GL_COMMAND_VERTEX,
	GL_COMMAND_TEXCOORD
};

typedef struct gl_color {
	GLdouble r;
	GLdouble g;
	GLdouble b;
	GLdouble a;
} gl_color;

typedef struct gl_vertex {
	GLdouble x;
	GLdouble y;
	GLdouble z;
	GLdouble w;
} gl_vertex;

typedef struct gl_texcoord {
	GLdouble s;
	GLdouble t;
	GLdouble r;
	GLdouble q;
} gl_texcoord;

typedef struct gl_command {
	enum gl_command_type type;
	union gl_command_info {
		gl_color color;
		gl_vertex vertex;
		gl_texcoord texcoord;
	} info;
} gl_command;

#define GL_CMDLIST_FLAG_ACTIVE 1

typedef struct gl_command_list {
	gl_command *cmds;
	size_t n_cmds;
	GLenum mode;
	char flags;
} gl_command_list;

typedef struct gl_context {
	gl_command_list curr_list; /* Current list on the context, for example glBegin() and glEnd() */
	gl_command_list lists[GL_MAX_LISTS];
	size_t n_lists;
	gl_color clear_color;
} gl_context;

static gl_context g_ctx;
static GLenum g_errcode = -1;

static gl_add_command(gl_command_list *list, gl_command cmd)
{
	list->cmds = realloc(list->cmds, list->n_cmds * sizeof(gl_command));
	if(list->cmds == NULL) {
		/* TODO: Set error */
	}
	list->n_cmds++;
}

GLAPI void glCullFace(GLenum mode)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glFrontFace(GLenum mode)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glHint(GLenum target, GLenum mode)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glLineWidth(GLfloat width)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glPointSize(GLfloat size)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glPolygonMode(GLenum face, GLenum mode)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glScissor(GLint x, GLint y, GLsizei width, GLsizei height)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexParameterf(GLenum target, GLenum pname, GLfloat param)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexParameteri(GLenum target, GLenum pname, GLint param)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexParameteriv(GLenum target, GLenum pname, const GLint *params)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const void *pixels)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *pixels)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glDrawBuffer(GLenum buf)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glClear(GLbitfield mask)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glClearStencil(GLint s)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glClearDepth(GLdouble depth)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glStencilMask(GLuint mask)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glDepthMask(GLboolean flag)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glDisable(GLenum cap)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glEnable(GLenum cap)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glFinish(void)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glFlush(void)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glBlendFunc(GLenum sfactor, GLenum dfactor)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glLogicOp(GLenum opcode)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glStencilFunc(GLenum func, GLint ref, GLuint mask)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glDepthFunc(GLenum func)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glPixelStoref(GLenum pname, GLfloat param)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glPixelStorei(GLenum pname, GLint param)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glReadBuffer(GLenum src)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, void *pixels)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glGetBooleanv(GLenum pname, GLboolean *data)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glGetDoublev(GLenum pname, GLdouble *data)
{
	GL_UNIMPLEMENTED();
}

GLAPI GLenum glGetError(void)
{
	return g_errcode;
}

GLAPI void glGetFloatv(GLenum pname, GLfloat *data)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glGetIntegerv(GLenum pname, GLint *data)
{
	GL_UNIMPLEMENTED();
}

GLAPI const GLubyte *glGetString(GLenum name)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, void *pixels)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glGetTexParameteriv(GLenum target, GLenum pname, GLint *params)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat *params)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint *params)
{
	GL_UNIMPLEMENTED();
}

GLAPI GLboolean glIsEnabled(GLenum cap)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glDepthRange(GLdouble n, GLdouble f)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glViewport(GLint x, GLint y, GLsizei width, GLsizei height)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glNewList(GLuint list, GLenum mode)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glEndList(void)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glCallList(GLuint list)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glCallLists(GLsizei n, GLenum type, const void *lists)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glDeleteLists(GLuint list, GLsizei range)
{
	GL_UNIMPLEMENTED();
}

GLAPI GLuint glGenLists(GLsizei range)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glListBase(GLuint base)
{
	GL_UNIMPLEMENTED();
}

/// @brief Begin the commands for a polygon
/// Reference: https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/glBegin.xml
/// @param mode 
GLAPI void glBegin(GLenum mode)
{
	/* glBegin() after a glBegin() */
	if(g_ctx.curr_list.flags & GL_CMDLIST_FLAG_ACTIVE) {
		g_errcode = GL_INVALID_OPERATION;
		return;
	}

	/* Validate the mode given as argument */
	switch(mode) {
	case GL_POINTS:
	case GL_LINES:
	case GL_LINE_STRIP:
	case GL_LINE_LOOP:
	case GL_TRIANGLES:
	case GL_TRIANGLE_STRIP:
	case GL_TRIANGLE_FAN:
	case GL_QUADS:
	case GL_QUAD_STRIP:
	case GL_POLYGON:
		break;
	default:
		g_errcode = GL_INVALID_ENUM; /* Error out */
		return;
	}

	g_ctx.curr_list.mode = mode;
	g_ctx.curr_list.flags |= GL_CMDLIST_FLAG_ACTIVE;
}

GLAPI void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte *bitmap)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glColor3b(GLbyte red, GLbyte green, GLbyte blue)
{
	glColor3d((GLfloat)red / CHAR_MAX, (GLfloat)green / CHAR_MAX, (GLfloat)blue / CHAR_MAX);
}

GLAPI void glColor3bv(const GLbyte *v)
{
	assert(v != NULL);
	glColor3b(v[0], v[1], v[2]);
}

GLAPI void glColor3d(GLdouble red, GLdouble green, GLdouble blue)
{
	glColor4d(red, green, blue, 1.0f);
}

GLAPI void glColor3dv(const GLdouble *v)
{
	assert(v != NULL);
	glColor3d(v[0], v[1], v[2]);
}

GLAPI void glColor3f(GLfloat red, GLfloat green, GLfloat blue)
{
	glColor3d((GLdouble)red, (GLdouble)green, (GLdouble)blue);
}

GLAPI void glColor3fv(const GLfloat *v)
{
	assert(v != NULL);
	glColor3f(v[0], v[1], v[2]);
}

GLAPI void glColor3i(GLint red, GLint green, GLint blue)
{
	glColor3d((GLfloat)red / INT_MAX, (GLfloat)green / INT_MAX, (GLfloat)blue / INT_MAX);
}

GLAPI void glColor3iv(const GLint *v)
{
	assert(v != NULL);
	glColor3i(v[0], v[1], v[2]);
}

GLAPI void glColor3s(GLshort red, GLshort green, GLshort blue)
{
	glColor3d((GLfloat)red / SHRT_MAX, (GLfloat)green / SHRT_MAX, (GLfloat)blue / SHRT_MAX);
}

GLAPI void glColor3sv(const GLshort *v)
{
	assert(v != NULL);
	glColor3s(v[0], v[1], v[2]);
}

GLAPI void glColor3ub(GLubyte red, GLubyte green, GLubyte blue)
{
	glColor3d((GLfloat)red / UCHAR_MAX, (GLfloat)green / UCHAR_MAX, (GLfloat)blue / UCHAR_MAX);
}

GLAPI void glColor3ubv(const GLubyte *v)
{
	assert(v != NULL);
	glColor3ub(v[0], v[1], v[2]);
}

GLAPI void glColor3ui(GLuint red, GLuint green, GLuint blue)
{
	glColor3d((GLfloat)red / UINT_MAX, (GLfloat)green / UINT_MAX, (GLfloat)blue / UINT_MAX);
}

GLAPI void glColor3uiv(const GLuint *v)
{
	assert(v != NULL);
	glColor3ui(v[0], v[1], v[2]);
}

GLAPI void glColor3us(GLushort red, GLushort green, GLushort blue)
{
	glColor3d((GLfloat)red / USHRT_MAX, (GLfloat)green / USHRT_MAX, (GLfloat)blue / USHRT_MAX);
}

GLAPI void glColor3usv(const GLushort *v)
{
	assert(v != NULL);
	glColor3us(v[0], v[1], v[2]);
}

GLAPI void glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha)
{
	glColor4d((GLdouble)red / CHAR_MAX, (GLdouble)green / CHAR_MAX, (GLdouble)blue / CHAR_MAX, (GLdouble)alpha / CHAR_MAX);
}

GLAPI void glColor4bv(const GLbyte *v)
{
	assert(v != NULL);
	glColor4b(v[0], v[1], v[2], v[3]);
}

GLAPI void glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha)
{
	gl_command cmd = {0};
	cmd.type = GL_COMMAND_COLOR;
	cmd.info.color.r = red;
	cmd.info.color.g = green;
	cmd.info.color.b = blue;
	cmd.info.color.a = alpha;
	gl_add_command(&g_ctx.curr_list, cmd);
}

GLAPI void glColor4dv(const GLdouble *v)
{
	assert(v != NULL);
	glColor4d(v[0], v[1], v[2], v[3]);
}

GLAPI void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
	glColor4d((GLdouble)red, (GLdouble)green, (GLdouble)blue, (GLdouble)alpha);
}

GLAPI void glColor4fv(const GLfloat *v)
{
	assert(v != NULL);
	glColor4f(v[0], v[1], v[2], v[3]);
}

GLAPI void glColor4i(GLint red, GLint green, GLint blue, GLint alpha)
{
	glColor4d((GLdouble)red / INT_MAX, (GLdouble)green / INT_MAX, (GLdouble)blue / INT_MAX, (GLdouble)alpha / INT_MAX);
}

GLAPI void glColor4iv(const GLint *v)
{
	assert(v != NULL);
	glColor4i(v[0], v[1], v[2], v[3]);
}

GLAPI void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha)
{
	glColor4d((GLdouble)red / SHRT_MAX, (GLdouble)green / SHRT_MAX, (GLdouble)blue / SHRT_MAX, (GLdouble)alpha / SHRT_MAX);
}

GLAPI void glColor4sv(const GLshort *v)
{
	assert(v != NULL);
	glColor4s(v[0], v[1], v[2], v[3]);
}

GLAPI void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
	glColor4d((GLdouble)red / UCHAR_MAX, (GLdouble)green / UCHAR_MAX, (GLdouble)blue / UCHAR_MAX, (GLdouble)alpha / UCHAR_MAX);
}

GLAPI void glColor4ubv(const GLubyte *v)
{
	assert(v != NULL);
	glColor4ub(v[0], v[1], v[2], v[3]);
}

GLAPI void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha)
{
	glColor4d((GLdouble)red / UINT_MAX, (GLdouble)green / UINT_MAX, (GLdouble)blue / UINT_MAX, (GLdouble)alpha / UINT_MAX);
}

GLAPI void glColor4uiv(const GLuint *v)
{
	assert(v != NULL);
	glColor4ui(v[0], v[1], v[2], v[3]);
}

GLAPI void glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha)
{
	glColor4d((GLdouble)red / USHRT_MAX, (GLdouble)green / USHRT_MAX, (GLdouble)blue / USHRT_MAX, (GLdouble)alpha / USHRT_MAX);
}

GLAPI void glColor4usv(const GLushort *v)
{
	assert(v != NULL);
	glColor4us(v[0], v[1], v[2], v[3]);
}

GLAPI void glEdgeFlag(GLboolean flag)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glEdgeFlagv(const GLboolean *flag)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glEnd(void)
{
	/* glEnd() executed before a glBegin() */
	if(g_ctx.curr_list.flags & GL_CMDLIST_FLAG_ACTIVE == 0) {
		g_errcode = GL_INVALID_OPERATION;
		return;
	}

	g_ctx.curr_list.flags &= (~GL_CMDLIST_FLAG_ACTIVE); /* Mark unactive */
	/* TODO: Draw the finished list */
	GL_UNIMPLEMENTED();
}

GLAPI void glIndexd(GLdouble c)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glIndexdv(const GLdouble *c)
{
	assert(c != NULL);
	glIndexd(c[0]);
}

GLAPI void glIndexf(GLfloat c)
{
	glIndexd((GLdouble)c);
}

GLAPI void glIndexfv(const GLfloat *c)
{
	assert(c != NULL);
	glIndexd((GLdouble)c[0]);
}

GLAPI void glIndexi(GLint c)
{
	glIndexd((GLdouble)c);
}

GLAPI void glIndexiv(const GLint *c)
{
	assert(c != NULL);
	glIndexd((GLdouble)c[0]);
}

GLAPI void glIndexs(GLshort c)
{
	glIndexd((GLdouble)c);
}

GLAPI void glIndexsv(const GLshort *c)
{
	assert(c != NULL);
	glIndexd((GLdouble)c[0]);
}

GLAPI void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz)
{
	glNormal3d((GLdouble)nx, (GLdouble)ny, (GLdouble)nz);
}

GLAPI void glNormal3bv(const GLbyte *v)
{
	assert(v != NULL);
	glNormal3b(v[0], v[1], v[2]);
}

GLAPI void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glNormal3dv(const GLdouble *v)
{
	assert(v != NULL);
	glNormal3d(v[0], v[1], v[2]);
}

GLAPI void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
	glNormal3d((GLdouble)nx, (GLdouble)ny, (GLdouble)nz);
}

GLAPI void glNormal3fv(const GLfloat *v)
{
	assert(v != NULL);
	glNormal3f(v[0], v[1], v[2]);
}

GLAPI void glNormal3i(GLint nx, GLint ny, GLint nz)
{
	glNormal3d((GLdouble)nx, (GLdouble)ny, (GLdouble)nz);
}

GLAPI void glNormal3iv(const GLint *v)
{
	assert(v != NULL);
	glNormal3i(v[0], v[1], v[2]);
}

GLAPI void glNormal3s(GLshort nx, GLshort ny, GLshort nz)
{
	glNormal3d((GLdouble)nx, (GLdouble)ny, (GLdouble)nz);
}

GLAPI void glNormal3sv(const GLshort *v)
{
	assert(v != NULL);
	glNormal3s(v[0], v[1], v[2]);
}

GLAPI void glRasterPos2d(GLdouble x, GLdouble y)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos2dv(const GLdouble *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos2f(GLfloat x, GLfloat y)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos2fv(const GLfloat *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos2i(GLint x, GLint y)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos2iv(const GLint *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos2s(GLshort x, GLshort y)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos2sv(const GLshort *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos3dv(const GLdouble *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos3fv(const GLfloat *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos3i(GLint x, GLint y, GLint z)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos3iv(const GLint *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos3s(GLshort x, GLshort y, GLshort z)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos3sv(const GLshort *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos4dv(const GLdouble *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos4fv(const GLfloat *v)
{
	assert(v != NULL);
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos4i(GLint x, GLint y, GLint z, GLint w)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos4iv(const GLint *v)
{
	assert(v != NULL);
	glRasterPos4i(v[0], v[1], v[2], v[3]);
}

GLAPI void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRasterPos4sv(const GLshort *v)
{
	assert(v != NULL);
	glRasterPos4s(v[0], v[1], v[2], v[3]);
}

GLAPI void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRectdv(const GLdouble *v1, const GLdouble *v2)
{
	assert(v1 != NULL && v2 != NULL);
	glRectd(v1[0], v1[2], v2[0], v2[1]);
}

GLAPI void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRectfv(const GLfloat *v1, const GLfloat *v2)
{
	assert(v1 != NULL && v2 != NULL);
	glRectf(v1[0], v1[2], v2[0], v2[1]);
}

GLAPI void glRecti(GLint x1, GLint y1, GLint x2, GLint y2)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRectiv(const GLint *v1, const GLint *v2)
{
	assert(v1 != NULL && v2 != NULL);
	glRecti(v1[0], v1[2], v2[0], v2[1]);
}

GLAPI void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glRectsv(const GLshort *v1, const GLshort *v2)
{
	assert(v1 != NULL && v2 != NULL);
	glRects(v1[0], v1[2], v2[0], v2[1]);
}

GLAPI void glTexCoord1d(GLdouble s)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord1dv(const GLdouble *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord1f(GLfloat s)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord1fv(const GLfloat *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord1i(GLint s)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord1iv(const GLint *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord1s(GLshort s)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord1sv(const GLshort *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord2d(GLdouble s, GLdouble t)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord2dv(const GLdouble *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord2f(GLfloat s, GLfloat t)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord2fv(const GLfloat *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord2i(GLint s, GLint t)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord2iv(const GLint *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord2s(GLshort s, GLshort t)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord2sv(const GLshort *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord3dv(const GLdouble *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord3fv(const GLfloat *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord3i(GLint s, GLint t, GLint r)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord3iv(const GLint *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord3s(GLshort s, GLshort t, GLshort r)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord3sv(const GLshort *v)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q)
{
	gl_command cmd = {0};
	cmd.type = GL_COMMAND_TEXCOORD;
	cmd.info.texcoord.s = s;
	cmd.info.texcoord.t = t;
	cmd.info.texcoord.r = r;
	cmd.info.texcoord.q = q;
	gl_add_command(&g_ctx.curr_list, cmd);
}

GLAPI void glTexCoord4dv(const GLdouble *v)
{
	assert(v != NULL);
	glTexCoord4d(v[0], v[1], v[2], v[3]);
}

GLAPI void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord4fv(const GLfloat *v)
{
	assert(v != NULL);
	glTexCoord4f(v[0], v[1], v[2], v[3]);
}

GLAPI void glTexCoord4i(GLint s, GLint t, GLint r, GLint q)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord4iv(const GLint *v)
{
	assert(v != NULL);
	glTexCoord4i(v[0], v[1], v[2], v[3]);
}

GLAPI void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glTexCoord4sv(const GLshort *v)
{
	assert(v != NULL);
	glTexCoord4s(v[0], v[1], v[2], v[3]);
}

GLAPI void glVertex2d(GLdouble x, GLdouble y)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex2dv(const GLdouble *v)
{
	assert(v != NULL);
	glVertex2d(v[0], v[1]);
}

GLAPI void glVertex2f(GLfloat x, GLfloat y)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex2fv(const GLfloat *v)
{
	assert(v != NULL);
	glVertex2f(v[0], v[1]);
}

GLAPI void glVertex2i(GLint x, GLint y)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex2iv(const GLint *v)
{
	assert(v != NULL);
	glVertex2i(v[0], v[1]);
}

GLAPI void glVertex2s(GLshort x, GLshort y)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex2sv(const GLshort *v)
{
	assert(v != NULL);
	glVertex2s(v[0], v[1]);
}

GLAPI void glVertex3d(GLdouble x, GLdouble y, GLdouble z)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex3dv(const GLdouble *v)
{
	assert(v != NULL);
	glVertex3d(v[0], v[1], v[2]);
}

GLAPI void glVertex3f(GLfloat x, GLfloat y, GLfloat z)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex3fv(const GLfloat *v)
{
	assert(v != NULL);
	glVertex3f(v[0], v[1], v[2]);
}

GLAPI void glVertex3i(GLint x, GLint y, GLint z)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex3iv(const GLint *v)
{
	assert(v != NULL);
	glVertex3i(v[0], v[1], v[2]);
}

GLAPI void glVertex3s(GLshort x, GLshort y, GLshort z)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex3sv(const GLshort *v)
{
	assert(v != NULL);
	glVertex3s(v[0], v[1], v[2]);
}

GLAPI void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex4dv(const GLdouble *v)
{
	assert(v != NULL);
	glVertex4d(v[0], v[1], v[2], v[3]);
}

GLAPI void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex4fv(const GLfloat *v)
{
	assert(v != NULL);
	glVertex4f(v[0], v[1], v[2], v[3]);
}

GLAPI void glVertex4i(GLint x, GLint y, GLint z, GLint w)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex4iv(const GLint *v)
{
	assert(v != NULL);
	glVertex4i(v[0], v[1], v[2], v[3]);
}

GLAPI void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w)
{
	GL_UNIMPLEMENTED();
}

GLAPI void glVertex4sv(const GLshort *v)
{
	assert(v != NULL);
	glVertex4s(v[0], v[1], v[2], v[3]);
}

