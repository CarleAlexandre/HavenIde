NAME		=	debug.exe
RELEASE		=	HavenIde.exe

BUILDDIR	=	bin/

CC			=	g++

CFLAGS		=	-g --std=c++20

OBJ			=	$(SRC:%.cpp=%.o)

DEPS		+=	$(wildcard include/*.h)

SRC			+=	$(wildcard source/*.cpp)

INCLUDE		=	-I include -I ./HavenLib/include

ifeq ($(OS), Windows_NT)
INCLUDE		+=	-I C:/mingw64/include
LIBS		=	-lopengl32 -lgdi32 -lwinmm -lstdc++ -latomic
RAYLIB		=	libs/win_libraylib.a
endif
ifeq ($(shell uname -s), Linux)
#CFLAGS		+=	-fsanitize=address -lasan
LIBS		=	-lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lstdc++ -latomic
endif
ifeq ($(shell uname -s), Darwin)
LIBS        =   -framework CoreVideo -framework IOKit -framework Coc
RAYLIB      =   libs/mac_libraylib.a
endif

RAYLIB		+=	-LHavenLib/build/ -lHavenLib

$(BUILDDIR)$(NAME): $(OBJ) $(DEPS)
		make -C HavenLib
		mkdir -p $(BUILDDIR)
		$(CC) $(OBJ) ${RAYLIB} ${LIBS} -o $@

$(BUILDDIR)$(RELEASE): $(OBJ) $(DEPS)
		make -C HavenLib
		mkdir -p $(BUILDDIR)
		$(CC) $(OBJ) ${RAYLIB} -mwindows -Wl,--subsystem,windows ${LIBS} -o $@

$(OBJ):		%.o :	%.cpp $(DEPS)
		$(CC) $(CFLAGS) ${INCLUDE} -c $< -o $@

all		:	debug

debug	:	$(BUILDDIR)$(NAME)


clean		:
		rm -f $(OBJ)

fclean		:	clean
		rm -f $(BUILDDIR)$(NAME)
		rm -f $(BUILDDIR)$(RELEASE)

re		:	fclean all
