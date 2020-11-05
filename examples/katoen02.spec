(system 2
        ; Init: start at 00
        (and (not (var 0)) (not (var 1)))
        ; Trans:
        (or (and (not (var from 0)) (not (var from 1))      (var to 0)  (not (var to 1))) ;00->10
            (and      (var from 0)  (not (var from 1)) (not (var to 0))      (var to 1) ) ;10->01
            (and (not (var from 0))      (var from 1)  (not (var to 0)) (not (var to 1))) ;01->00
            (and (not (var from 0))      (var from 1)       (var to 0)       (var to 1) ) ;01->11
            (and      (var from 0)       (var from 1)  (not (var to 0))      (var to 1) ) ;11->01
        )
                                                    ; The following pertain to the non-fair case:
        (properties (AG (AF (var 0)))               ; True, var0 is always hit infinitely often
                    (AF (EG (var 1)))               ; True, we always eventually reach 01, from
                                                    ; where there is a path where v1 is always true
                    (AU (not (var 1)) (EG (var 1))) ; True, for all paths where v0 does not hold
                                                    ; until 01, from where we have a path where v1
                                                    ; always holds.
                    (EF (AG (var 1)))               ; False, no point is such that from there all paths
                                                    ; have v1 as true
                    (EU (not (var 0)) (EG (var 1))) ; False, for all paths starting from 00, v0 is
                                                    ; false for the first time at 10, where no path
                                                    ; is such that from there v1 always holds
                    (AF (and (var 0) (var 1)))      ; This is not true, clearly
        )
        ; Uncomment one of the fairness conditins below:
        ;((and (var 0) (not (var 1))))               ; just 10 is fair Under this, properties should
                                                    ; be 1 - true, 2 - false, 3 - false, 4 - false,
                                                    ; 5 - false, 6 - false
        ;((and (var 0) (not (var 1)))                ; Under this, the situation is the same as
        ; (and (var 0)      (var 1)))                ; before, but the last one is now sat.

        ((and (not (var 0)) (var 1)))               ; Here, all paths are fair, and thus the
                                                    ; situation should be identical to the unfair
                                                    ; case
)

