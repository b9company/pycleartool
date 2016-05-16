/*************************************************************************
*                                                                        *
* � Copyright IBM Corporation 2001, 2004. All rights reserved.           *
*                                                                        *
* This program and the accompanying materials are made available under   *
* the terms of the Common Public License v1.0 which accompanies this     *
* distribution, and is also available at http://www.opensource.org       *
*                                                                        *
* Contributors:                                                          *
*                                                                        *
* William Spurlin - Initial version and framework                        *
* Vincent Besanceney - Changes and fixes for Pycleartool extension       *
*                                                                        *
*************************************************************************/


/* Vincent Besanceney: Provided macro definitions for NULL, TRUE, or FALSE for
 * Python. */
#ifdef NULL
#undef NULL
#endif
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#define NULL 0
#define TRUE (1)
#define FALSE (0)

#define T_OK 0 

typedef  struct gen_t_struct * gen_t ;
typedef  enum {A,B,C,D} enum_t;
typedef  int (*gen_2_t)(gen_t,gen_t,enum_t);


typedef struct {
    char *buffP;
    int currSize;
    int buffSize;
} BLOK;

#define STANDARD (BLOK*) 0     
#define DEVNULL  (BLOK*) 1     
#define BLOK_START_SIZE 4096



#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif 
extern  int (*cmdsyn_proc_table[])(gen_t,gen_t,enum_t);
#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif 
extern gen_t * cmdsyn_get_cmdflags();

int client_meters_enabled = FALSE;
void client_meters_finish_program(void){int empty = 1;}
gen_t client_meters_create_region(const char *name){
  int empty = 1;
  return (gen_t)empty;
}
void client_meters_exit_region(gen_t handle){int empty = 1;}
void client_meters_enter_region(gen_t handle){int empty = 1;}



#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif 
extern void imsg_set_app_name (char *);
#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif 
extern void imsg_redirect_output (void (*)(void *,char *), BLOK *, void (*)(void *,char *), BLOK *);
#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif 
extern void stg_free_area (gen_t, int);
#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif 
extern int cmdsyn_exec_dispatch (char*, gen_t,gen_t *, 
                          int (*[])(gen_t,gen_t,enum_t));
#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif 
extern int cmdsyn_execv_dispatch (int,char **,gen_t,gen_t *, 
                          int (*[])(gen_t,gen_t,enum_t));
#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif 
extern gen_t stg_create_area ( int );
#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif
extern int vob_ob_all_cache_action(int, int, int);
#ifdef ATRIA_WIN32_COMMON
__declspec( dllimport )
#endif
/* Vincent Besanceney: Renamed pfm_int() to pfm_init(). */
extern void pfm_init(void);
/* Vincent Besanceney: Moved dispatched_synv_call() and dispatched_syn_call()
 * to cleartool.c. */
