#!/bin/bash
# -*- scheme -*-
exec guile -l $0 -e main "$@"
!#

;; GStreamer
;; Copyright (C) 2005 Andy Wingo <wingo at pobox.com>

;; This program is free software; you can redistribute it and/or    
;; modify it under the terms of the GNU General Public License as   
;; published by the Free Software Foundation; either version 2 of   
;; the License, or (at your option) any later version.              
;;                                                                  
;; This program is distributed in the hope that it will be useful,  
;; but WITHOUT ANY WARRANTY; without even the implied warranty of   
;; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    
;; GNU General Public License for more details.                     
;;                                                                  
;; You should have received a copy of the GNU General Public License
;; along with this program; if not, contact:
;;
;; Free Software Foundation           Voice:  +1-617-542-5942
;; 59 Temple Place - Suite 330        Fax:    +1-617-542-2652
;; Boston, MA  02111-1307,  USA       gnu@gnu.org


;;; Commentary:
;;
;; Network clock simulator.
;;
;; Simulates the attempts of one clock to synchronize with another over
;; the network. Packets are sent out with a local timestamp, and come
;; back with the remote time added on to the packet. The remote time is
;; assumed to have been observed at the local time in between sending
;; the query and receiving the reply.
;;
;; The local clock will attempt to adjust its rate and offset by fitting
;; a line to the last N datapoints on hand, by default 32. A better fit,
;; as measured by the correlation coefficient, will result in a longer
;; time before the next query. Bad fits or a not-yet-full set of data
;; will result in many queries in quick succession.
;;
;; The rate and offset are set directly to the slope and intercept from
;; the linear regression. This results in discontinuities in the local
;; time. As clock times must be monotonically increasing, a jump down in
;; time will result instead in time standing still for a while. Smoothly
;; varying the rate such that no discontinuities are present has not
;; been investigated.
;;
;; Implementation-wise, this simulator processes events and calculates
;; times discretely. Times are represented as streams, also known as
;; lazy lists. This is an almost-pure functional simulator. The thing to
;; remember while reading is that stream-cons does not evaluate its
;; second argument, rather deferring that calculation until stream-cdr
;; is called. In that way all times are actually infinite series.
;;
;; Knobs: sample rate, send delay, receive delay, send noise, receive
;; noise, queue length, rate of remote clock, rate of local clock.
;; Fixme: Make knobs more accesible tomorrow; also make graphs.
;;
;;; Code:


(use-modules (ice-9 slib))
(require 'printf)

(load "network-clock-utils.scm")


(define *sample-frequency* 40)

(define (time->samples t)
  (iround (* t *sample-frequency*)))

(define *absolute-time* (arithmetic-series 0.0 (/ 1.0 *sample-frequency*)))

(define *empty-event-stream* (stream-of #f))

(define (schedule-event events e time)
  (let lp ((response-time (time->samples time))
           (stream events))
    (if (zero? response-time)
        (if (not (stream-car stream))
            (stream-cons e (stream-cdr stream))
            (stream-cons (stream-car stream) (lp 0 (stream-cdr stream))))
        (stream-cons (stream-car stream) (lp (1- response-time) (stream-cdr stream))))))

(define (schedule-send-time-query events time)
  (schedule-event events (list 'send-time-query) time))

(define (schedule-time-query events l)
  (schedule-event events (list 'time-query l) (+ 0.20 (random 0.20))))

(define (schedule-time-response events l r)
  (schedule-event events (list 'time-response l r) (+ 0.20 (random 0.20))))

(define (network-time remote-time local-time events m b x y)
  (let ((r (stream-car remote-time))
        (l (stream-car local-time))
        (event (stream-car events))
        (events (stream-cdr events)))

    (define (next events m b x y)
      (stream-cons
       (+ (* m l) b)
       (network-time
        (stream-cdr remote-time) (stream-cdr local-time) events m b x y)))

    (case (and=> event car)
      ((send-time-query)
       (format #t "sending time query: ~a\n" l)
       (next (schedule-time-query events l) m b x y))

      ((time-query)
       (format #t "time query received, replying with ~a\n" r)
       (next (schedule-time-response events (cadr event) r) m b x y))

      ((time-response)
       (let ((x (q-push x (avg (cadr event) l)))
             (y (q-push y (caddr event))))
         (call-with-values
             (lambda () (least-squares (q-head x) (q-head y)))
           (lambda (m b r-squared)
             (define (next-time) 
               (max
                (if (< (length (q-head x)) *q-length*)
                    0
                    (/ 1 (- 1 (min r-squared 0.99999)) 1000))
                0.10))
             (format #t "new slope and offset: ~a ~a (~a)\n" m b r-squared)
             (next (schedule-send-time-query events (next-time)) m b x y)))))

      (else
       (next events m b x y)))))

(define (run-simulation remote-speed local-speed)
  (let ((remote-time (scale-stream *absolute-time* remote-speed))
        (local-time (scale-stream *absolute-time* local-speed)))
    (values
     *absolute-time*
     remote-time
     local-time
     (network-time
      remote-time
      local-time
      (schedule-send-time-query *empty-event-stream* 0.0)
      1.0
      (stream-car local-time)
      (make-q (list (stream-car local-time)))
      (make-q (list (stream-car remote-time)))))))

(define (print-simulation total-time sample-rate remote-speed local-speed)
  (display ";; absolute remote local network\n")
  (call-with-values
      (lambda () (run-simulation remote-speed local-speed))
    (lambda streams
      (apply
       stream-while
       (lambda (a r l n) (<= a total-time))
       (lambda (a r l n) (printf "%.3f %.3f %.3f %.3f\n" a r l n))
       streams))))

(define (main . args)
  (print-simulation 20 #f 2.0 1.1))