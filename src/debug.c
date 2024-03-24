#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "domain_state.h"
#include "mlvalues.h"

void print_stack(){
    printf("______________________________\n");
    printf("|    addresse    | valeur | block? |\n");
    for(int i=Caml_state->sp;i>0;i--){
        mlvalue *addr = (Caml_state->stack+i-1);
        mlvalue  content = *addr ;
        char* is_block = (Is_block(content)?"yes":"no"); 
        if(Is_block(content))
        printf("| %p | %p | %s |\n",addr,content,is_block);
        
    }
    
}

void print_heap(){
    printf("______________________________\n");
    printf("| addresse block | valeur | head? | color? | taille |\n");
    for(int i=0;(i*sizeof(mlvalue))<Caml_state->heap_size;i++){
        
        mlvalue *addr = (Caml_state->heap+i);
        mlvalue  content = *addr ;
        char* couleur = ((Color_hd(content)==WHITE)?"white":"black");
        int size = Size_hd(content);
        //if(Color_hd(content)==BLACK){
        printf("\n| %p | %ld | yes | %s | %d | i : %d size_heap : %d \n",addr,content,couleur,size,i,Caml_state->heap_size);
        printf("| addresse | valeur | head? | is_block? | \n");
        for(int j = 0;j<size;j++){
            mlvalue *addr = (Caml_state->heap+i+j);
            mlvalue  content = *addr ;
            char* is_block = (Is_block(content)?"yes":"no"); 
            if(Is_block(content)){
                printf("| %p | %p | no | %s | \n",addr,content,is_block); 
            }else{
                printf("| %p | %ld | no | %s | \n",addr,content,is_block); 
            }
        }
        
        //}
        i+=size;
        
        
    }
}