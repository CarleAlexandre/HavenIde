NAME		=	HavenIde

BUILDDIR	=	build/

CC			=	gcc

CFLAGS		=	-g

OBJ			=	$(SRC:%.cpp=%.o)

DEPS		+=	$(wildcard include/*.h)

SRC			+=	$(wildcard source/*.cpp)

INCLUDE		=	-I include -I HavenLib/include

ifeq ($(OS), Windows_NT)
INCLUDE		+=	-I C:/mingw64/include
LIBS		=	-lopengl32 -lgdi32 -lwinmm -lstdc++ -latomic
RAYLIB		=	libs/win_libraylib.a
endif
ifeq ($(shell uname -s), Linux)
CFLAGS		+=	-fsanitize=address
LIBS		=	-lasan -lraylib -lGL -lm -lpthread -ldl -lrt -lX11 -lstdc++
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

#add this above after lib to have no console: <-mwindows -Wl,--subsystem,windows>

$(OBJ):		%.o :	%.cpp $(DEPS)
		$(CC) $(CFLAGS) ${INCLUDE} -c $< -o $@

all		:	$(BUILDDIR)$(NAME)

clean		:
		rm -rf $(OBJ)

fclean		:	clean
		rm -rf $(BUILDDIR)

re		:	fclean all
