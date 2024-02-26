(* Goal: create a lot of temporary lists that can be freed *)
(* Note: without GC; with the naive version of the VM, this will
   probably use about 4.5GB or memory (ie, if you have less than that,
   change the "250" line 29 by something else) . *)

let rec make_list_0 size acc =
  if size = 0 then acc
  else make_list_0 (size - 1) (0 :: acc)
let make_list_0 size = make_list_0 size []

let rec map f l aux =
  match l with
  | [] -> aux
  | hd :: tl -> map f tl ((f hd) :: aux)
let map f l = map f l []

let rec length l acc =
  match l with
  | [] -> acc
  | _ :: tl -> (length tl (acc + 1))
let length l = length l 0

let add_1 x = x + 1

let rec map_add_1 l n =
  if n = 0 then l
  else map_add_1 (map add_1 l) (n-1)

let _ = length (map_add_1 (make_list_0 100000) 250)
