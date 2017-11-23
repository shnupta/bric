#ifndef _SYNTAX_DOCKERFILE_H
#define _SYNTAX_DOCKERFILE_H

// DOCKERFILE
char *DOCKERFILE_extensions[] = {"Dockerfile", NULL};
char *DOCKERFILE_keywords[] = {

	// Keyword: Misc	
	"FROM@", 
    "MAINTAINER",
    "RUN~", "CMD~",
    "ADD", "COPY",
    "EXPOSE", "ENV",
    "ENTRYPOINT",
    "VOLUME#", "USER#", "WORKDIR#", "ARG#",
    "ONBUILD^", "STOPSIGNAL^", "HEALTHCHECK^", "SHELL^",
    "LABEL",
    
	NULL
};

#define DOCKERFILE_syntax {	\
	DOCKERFILE_extensions,	\
	DOCKERFILE_keywords, 		\
	"#",				\
	"#", 				\
	"#", 				\
	HL_HIGHLIGHT_NUMBERS | HL_HIGHLIGHT_STRINGS \
}

#endif
