// CrossWorks Tasking Library.
//
// Copyright (c) 2004-2011 Rowley Associates Limited.
//
// This file may be distributed under the terms of the License Agreement
// provided with this software.
//
// THIS FILE IS PROVIDED AS IS WITH NO WARRANTY OF ANY KIND, INCLUDING THE
// WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

#ifndef CrossWorks_Tasking_Library
#define CrossWorks_Tasking_Library

#ifdef __cplusplus
extern "C" {
#endif

//
// Task Support
//

/*! \brief Task states \ingroup Types \synopsis

  \desc \b \this defines the states the task can be on.

  \table{1.5in,*}
  || Constant | Description
  || CTL_STATE_RUNNABLE |
       Task can run.
  || CTL_STATE_TIMER_WAIT |
       Waiting for a time value.
  || CTL_STATE_EVENT_WAIT_ALL |
       Waiting for all events to be set.
  || CTL_STATE_EVENT_WAIT_ALL_AC |
       Waiting for all events to be set with auto clear.
  || CTL_STATE_EVENT_WAIT_ANY |
       Waiting for any events to be set.
  || CTL_STATE_EVENT_WAIT_ANY_AC |
       Waiting for any events to be set with auto clear.
  || CTL_STATE_SEMAPHORE_WAIT |
       Task is waiting for a semaphore.
  || CTL_STATE_MESSAGE_QUEUE_POST_WAIT |
       Task is waiting to post to a message queue.
  || CTL_STATE_MESSAGE_QUEUE_RECEIVE_WAIT |
       Task is waiting to receive from a message queue.
  || CTL_STATE_MUTEX_WAIT |
      Task is waiting for a mutex.
  || CTL_STATE_SUSPENDED |
      Task cannot run.
  \endtable
*/
typedef enum
{
  CTL_STATE_RUNNABLE                   = 0x00, // Can run
  CTL_STATE_TIMER_WAIT                 = 0x01, // Waiting for a time value
  CTL_STATE_EVENT_WAIT_ALL             = 0x02, // Waiting for all events to be set
  CTL_STATE_EVENT_WAIT_ALL_AC          = 0x04, // Waiting for all events to be set with auto clear
  CTL_STATE_EVENT_WAIT_ANY             = 0x06, // Waiting for any events to be set
  CTL_STATE_EVENT_WAIT_ANY_AC          = 0x08, // Waiting for any events to be set with auto clear
  CTL_STATE_SEMAPHORE_WAIT             = 0x0A, // Task is waiting for a semaphore
  CTL_STATE_MESSAGE_QUEUE_POST_WAIT    = 0x0C, // Task is waiting to post to a message queue
  CTL_STATE_MESSAGE_QUEUE_RECEIVE_WAIT = 0x0E, // Task is waiting to receive from a message queue
  CTL_STATE_MUTEX_WAIT                 = 0x10, // Task is waiting for a mutex
  CTL_STATE_SUSPENDED                  = 0x80  // Task cannot run
} CTL_STATE_t;


/*! \group Types */

/*! \brief Time definition \ingroup Types \synopsis

  \desc \b \this defines the base type for times that CTL uses.
*/
typedef unsigned long CTL_TIME_t;


/*! \brief Event set definition \ingroup Types \synopsis
  
  \desc \b \this defines an event set. Event sets are word sized 16 or 32
  depending on the machine.
*/
typedef unsigned CTL_EVENT_SET_t;

/*! \endgroup Types */

/*! \group Tasks */

/*! \brief Task struct definition \ingroup Types \synopsis
  
  \desc \b \this defines the task structure. The task structure contains:

  \table{1.5in,*}
    || Member            | Description
    || stack_pointer     | the saved register state of the task when it is not scheduled
    || priority          | the priority of the task
    || state             | the state of task CTL_STATE_RUNNABLE or (CTL_STATE_*_WAIT_* \| CTL_STATE_TIMER_WAIT) or CTL_STATE_SUSPENDED
    || timeout_occured   | 1 if a wait timed out otherwise 0 - when state is CTL_RUNNABLE
    || next              | next pointer for wait queue
    || timeout           | wait timeout value or time slice value when the task is executing
    || wait_object       | the event set, semaphore, message queue or mutex to wait on
    || wait_events       | the events to wait for
    || thread_errno      | thread specific errno
    || data              | task specific data pointer
    || execution_time    | number of ticks the task has executed for   
    || stack_start       | the start (lowest address) of the stack 
    || name              | task name
  \endtable
*/
typedef struct CTL_TASK_s
{
  unsigned *stack_pointer; // don't move this - assembly code knows about it
  unsigned char priority;
  unsigned char state;
  unsigned char timeout_occured;
  struct CTL_TASK_s *next;                  
  CTL_TIME_t timeout;
  void *wait_object;          
  CTL_EVENT_SET_t wait_events;
  int thread_errno;
  void *data;
  CTL_TIME_t execution_time;
  unsigned *stack_start;
  const char *name;
} CTL_TASK_t;


/*! \brief Create the initial task \synopsis

  \desc \b \this turns the main program into a task. This function takes
  a pointer in \a task to the \b CTL_TASK_t structure that represents
  the main task, its \a priority (0 is the lowest priority, 255 the highest),
  and a zero-terminated string pointed by \a name. On return from this function
  global interrupts will be enabled.

  \notes The function must be called before any other CrossWorks tasking library calls
  are made.
*/
void ctl_task_init(CTL_TASK_t *task,
                   unsigned char priority,
                   const char *name);


/*! \brief Start a task \synopsis

  \desc \b \this takes a pointer in \a task to the \b CTL_TASK_t structure
  that represents the task. The \a priority can be zero for the lowest priority
  up to 255 which is the highest. The \a entrypoint parameter is the function
  that the task will execute which has the \a parameter passed to it.

  \a name is a pointer to a zero-terminated string used for debug purposes.

  The start of the memory used to hold the stack that the task will execute
  in is \a stack and the size of the memory is supplied in \a stack_size_in_words.
  On systems that have two stacks (e.g. Atmel AVR) then the \a call_size_in_words
  parameter must be set to specify the number of stack elements to use for the
  call stack.
*/
void ctl_task_run(CTL_TASK_t *task,
                  unsigned char priority,
                  void (*entrypoint)(void *),     // entrypoint the task starts executing - parameter is passed to it
                  void *parameter,                // parameter to pass to entrypoint
                  const char *name,
                  unsigned stack_size_in_words,   // note that the stack should be allocated in words not bytes
                  unsigned *stack,
                  unsigned call_size_in_words);   // optional parameter if call stack is separate from data stack


/*! \brief Remove a task from the task list \synopsis

  \desc \b \this removes the task \a task from the waiting task 
  list. Once you you have removed a task the only way to re-introduce it to the 
  system is to call \b ctl_task_restore.
*/
void ctl_task_remove(CTL_TASK_t *task);


/*! \brief Put back a task on to the task list \synopsis

  \desc \b \this adds a task \a task that was removed (using \b ctl_task_remove) onto the task list and 
  do scheduling.
*/
void ctl_task_restore(CTL_TASK_t *task);


/*! \brief Terminate the executing task \synopsis

  \desc \b \this terminates the currently executing task and schedules 
  the next runnable task.
*/
void ctl_task_die(void);


/*! \brief Set the priority of a task \synopsis

  \desc \b \this changes the priority of \a task to \a priority. 
  The priority can be 0, the lowest priority, to 255, which is the highest priority.

  You can change the priority of the currently executing task by passing
  \b ctl_task_executing as the \a task parameter.
  
  \b \this returns the previous priority of the task.
*/
unsigned char ctl_task_set_priority(CTL_TASK_t *task,
                                    unsigned char priority);


/*! \brief Cause a reschedule \synopsis

  \desc \b \this causes a reschedule to occur. This can be used by 
  tasks of the same priority to share the CPU without using timeslicing.
*/
void ctl_task_reschedule(void);


/*! \endgroup */
/*! \group Timer */

//
// Timer support
//


/*! \brief Atomically return the current time \synopsis

  \desc \b \this atomically reads the value of \b ctl_current_time. 
*/
CTL_TIME_t ctl_get_current_time(void);


/*! \brief Wait until timeout has occurred \synopsis

  \desc \b \this takes the \a timeout (not the duration) 
  as a parameter and suspends the calling task until the current time reaches
  the timeout.

  \note \b \this must not be called from an interrupt service routine.
*/
void ctl_timeout_wait(CTL_TIME_t timeout);


/*! \brief Increment tick timer \synopsis

  \desc \b \this increments \b ctl_current_time by
  the number held in \b ctl_time_increment and does rescheduling. 
  This function should be called from a periodic interrupt service 
  routine.

  \note \b \this must only be invoked by an interrupt service routine.
*/
void ctl_increment_tick_from_isr(void);

/*! \endgroup */


/*! \brief Type of wait \ingroup Types \synopsis

  \desc \b \this defines the type of timeout for
  a blocking function call.

  \table{1.5in,*}
  || Constant | Description
  || CTL_TIMEOUT_NONE |
        No timeout \-- block indefinitely.
  || CTL_TIMEOUT_INFINITE |
        Identical to \b CTL_TIMEOUT_NONE.
  || CTL_TIMEOUT_ABSOLUTE |
        The timeout is an absolute time.
  || CTL_TIMEOUT_DELAY |
        The timeout is relative to the current time.
  || CTL_TIMEOUT_NOW |
        The timeout happens immediately \-- no rescheduling occurs.
  \endtable
*/
typedef enum
{
  CTL_TIMEOUT_NONE,     // no timeout - block indefinitely
  CTL_TIMEOUT_INFINITE = CTL_TIMEOUT_NONE,
  CTL_TIMEOUT_ABSOLUTE, // the timeout is an absolute time
  CTL_TIMEOUT_DELAY,    // the timeout is a delay
  CTL_TIMEOUT_NOW       // the timeout happens immediately - no rescheduling occurs
} CTL_TIMEOUT_t;

#define CTL_TIMEOUT_INFINITE CTL_TIMEOUT_NONE

//
// Event support
//
/*! \group Event sets */

/*! \brief Initialize an event set \synopsis

  \desc \b \this initializes the event set \a e with the \a set values.
*/
void ctl_events_init(CTL_EVENT_SET_t *e,
                     CTL_EVENT_SET_t set);


/*! \brief Set and clear events in an event set \synopsis

  \desc \b \this sets the events defined by \a set and clears the events defined 
  by \a clear of the event set pointed to by \a e.  \b \this
  will then search the task list, matching tasks that are waiting on the event set \a e 
  and make them runnable if the match is successful.

  \sa \ref ctl_events_pulse.
*/
void ctl_events_set_clear(CTL_EVENT_SET_t *e,
                          CTL_EVENT_SET_t set,
                          CTL_EVENT_SET_t clear);


/*! \brief Pulse events in an event set \synopsis

  \desc \b \this will set the events defined by \a set_then_clear in the event
  set pointed to by \a e.  \b \this will then search the task list, matching tasks
  that are waiting on the event set \a e, and make them runnable if the match
  is successful. The events defined by \a set_then_clear are then
  cleared.

  \sa \ref ctl_events_set_clear.
*/
void ctl_events_pulse(CTL_EVENT_SET_t *e,
                      CTL_EVENT_SET_t set_then_clear);


/*! \brief Event set wait types \ingroup Types \synopsis

  \desc \b \this defines how to wait for an event set.

  \table{1.5in,*}
  || Constant | Description
  || CTL_EVENT_WAIT_ANY_EVENTS |
        Wait for any of the specified events to be set in the event set.
  || CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR |
        Wait for any of the specified events to be set in the event set
        and reset (clear) them.
  || CTL_EVENT_WAIT_ALL_EVENTS |
        Wait for all of the specified events to be set in the event set.
  || CTL_EVENT_WAIT_ALL_EVENTS_WITH_AUTO_CLEAR |
        Wait for all of the specified events to be set in the event set
        and reset (clear) them.
  \endtable

  \sa \ref ctl_events_wait
*/
typedef enum
{
  CTL_EVENT_WAIT_ANY_EVENTS = CTL_STATE_EVENT_WAIT_ANY,
  CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR = CTL_STATE_EVENT_WAIT_ANY_AC,
  CTL_EVENT_WAIT_ALL_EVENTS = CTL_STATE_EVENT_WAIT_ALL,
  CTL_EVENT_WAIT_ALL_EVENTS_WITH_AUTO_CLEAR = CTL_STATE_EVENT_WAIT_ALL_AC
} CTL_EVENT_WAIT_TYPE_t;


/*! \brief Wait for events in an event set \synopsis

  \desc \b \this waits for \a events to be 
  set (value 1) in the event set pointed to by \a eventSet with an optional 
  \a timeout applied if \a timeoutType is non-zero.

  The \a waitType can be one of:

  \item \b CTL_EVENT_WAIT_ANY_EVENTS \-- wait for any of \a events
    in \a eventSet to be set.
  \item \b CTL_EVENT_WAIT_ANY_EVENTS_WITH_AUTO_CLEAR \-- wait for any of 
    \a events in \a eventSet to be set and reset (clear) them.
  \item \b CTL_EVENT_WAIT_ALL_EVENTS \-- wait for all \a events
    in \b{*eventSet} to be set.
  \item \b CTL_EVENT_WAIT_ALL_EVENTS_WITH_AUTO_CLEAR \-- wait for all
    \a events in \b eventSet to be set and reset (clear) them.

  \b \this returns the value pointed to by \a eventSet
  before any auto-clearing occurred or zero if the \a timeout occurred.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_events_wait(CTL_EVENT_WAIT_TYPE_t type,
                         CTL_EVENT_SET_t *eventSet,
                         CTL_EVENT_SET_t events,
                         CTL_TIMEOUT_t t,
                         CTL_TIME_t timeout);

/*! \endgroup */


//
// Semaphore support
//
/*! \group Semaphores */


/*! \brief Semaphore definition \ingroup Types \synopsis

  \desc \b \this defines the semaphore type.
  Semaphores are held in one word, 16 or 32 bits depending on
  the machine.
*/
typedef unsigned CTL_SEMAPHORE_t;


/*! \brief Initialize a semaphore \synopsis

  \desc \b \this initializes the semaphore pointed to by \a s
  to \a value.
*/
void ctl_semaphore_init(CTL_SEMAPHORE_t *s,
                        unsigned value);


/*! \brief Signal a semaphore \synopsis

  \desc \b \this signals the semaphore pointed to by \a s. 
  If tasks are waiting for the semaphore then the highest priority task will be 
  made runnable. If no tasks are waiting for the semaphore then the semaphore 
  value will be incremented.
*/
void ctl_semaphore_signal(CTL_SEMAPHORE_t *s);


/*! \brief Wait for a semaphore \synopsis

  \desc \b \this waits for the semaphore pointed to by \a s
  to be non-zero. If the semaphore is zero then the caller will block
  unless \a timeoutType is non-zero and the current time reaches the \a timeout
  value. If the timeout occurred \b \this returns zero otherwise
  it returns one.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_semaphore_wait(CTL_SEMAPHORE_t *s,
                            CTL_TIMEOUT_t t,
                            CTL_TIME_t timeout);

/*! \endgroup */

//
// Message queue support.
//
/*! \group Message queues */

/*! \brief Message queue struct definition \ingroup Types \synopsis
  
  \desc \b \this defines the message queue structure. The message queue structure contains:

  \table{1.5in,*}
  || Member            | Description
  || q                 | pointer to the array of message queue objects
  || s                 | size of the array of message queue objects
  || front             | the next element to leave the message queue
  || n                 | the number of elements in the message queue
  || e                 | the event set to use for the not empty and not full events
  || notempty          | the event number for a not empty event
  || notfull           | the event number for a not full event
  \endtable
*/

typedef struct {
  void **q;
  unsigned s;
  unsigned front;
  unsigned n;
  CTL_EVENT_SET_t *e;
  CTL_EVENT_SET_t notempty;
  CTL_EVENT_SET_t notfull;
} CTL_MESSAGE_QUEUE_t;


/*! \brief Initialize a message queue \synopsis

  \desc \b \this is given a pointer to 
  the message queue to initialize in \a q. The array that will be 
  used to implement the message queue pointed to by \a queue and its size 
  in \a queue_size are also supplied.
*/
void ctl_message_queue_init(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                            void **queue,           // pointer to the queue
                            unsigned queue_size);   // the number of elements in the queue


/*! \brief Associate events with the not-full and not-empty state of a message queue \synopsis

  \desc \b \this registers events in the event set \a e that are set
  when the message queue \a q becomes \a notempty or becomes \a notfull.
  No scheduling will occur with this operation, you need to do this
  before waiting for events.
*/
void ctl_message_queue_setup_events(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                    CTL_EVENT_SET_t *e, // event set
                                    CTL_EVENT_SET_t notempty,
                                    CTL_EVENT_SET_t notfull); // event set on not_empty


/*! \brief Return number of free elements in a message queue \synopsis

  \desc \b \this returns the number of free elements in the message queue \a q.
*/
unsigned ctl_message_queue_num_free(CTL_MESSAGE_QUEUE_t *q);


/*! \brief Return number of used elements in a message queue \synopsis

  \desc \b \this returns the number of used elements in the message queue \a q.
*/
unsigned ctl_message_queue_num_used(CTL_MESSAGE_QUEUE_t *q);


/*! \brief Post message to a message queue \synopsis

  \desc \b \this posts \a message to the message queue pointed to
  by \a q. If the message queue is full then the caller will block until
  the message can be posted or, if \a timeoutType is non-zero, the
  current time reaches \a timeout value.
  
  \b \this returns zero if the timeout occurred otherwise it returns one.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_message_queue_post(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                void *message,          // the message to post
                                CTL_TIMEOUT_t t,        // type of timeout
                                CTL_TIME_t timeout);    // timeout


/*! \brief Post message to a message queue without blocking \synopsis

  \desc \b \this posts \a message to the message queue pointed to
  by \a q. If the message queue is full then the function will return
  zero otherwise it will return one.
*/
unsigned ctl_message_queue_post_nb(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                   void *message);         // the message to post


/*! \brief Receive message from a message queue \synopsis

  \desc \b \this pops the oldest message in the message queue pointed to
  by \a q into the memory pointed to by \a message. \b \this
  will block if no messages are available unless \a timeoutType
  is non-zero and the current time reaches the \a timeout value. 

  \b \this returns zero if a timeout occurs otherwise 1.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_message_queue_receive(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                   void **message,         // pointer to message receiver
                                   CTL_TIMEOUT_t t,        // type of timeout
                                   CTL_TIME_t timeout);    // timeout


/*! \brief Receive message from a message queue without blocking \synopsis

  \desc \b \this pops the oldest message in the message queue pointed to
  by \a q into the memory pointed to by \a message. If no messages are available 
  the function returns zero otherwise it returns 1.
*/
unsigned ctl_message_queue_receive_nb(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                      void **message);        // pointer to message receiver


/*! \brief Post messages to a message queue \synopsis

  \desc \b \this posts \a n \a messages to the message queue pointed to
  by \a q. The caller will block until
  the messages can be posted or, if \a timeoutType is non-zero, the
  current time reaches \a timeout value.
  
  \b \this returns the number of messages that were posted.

  \note \b \this must not be called from an interrupt service routine.

  \b \this function does not guarantee that the messages will be all be posted to the message queue atomically. 
  If you have multiple tasks posting (multiple messages) to the same message queue then you may get unexpected results.
*/
unsigned ctl_message_queue_post_multi(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                      unsigned n,             // the number of messages to post
                                      void **messages,        // the messages to post
                                      CTL_TIMEOUT_t t,        // type of timeout
                                      CTL_TIME_t timeout);    // timeout


/*! \brief Post messages to a message queue without blocking \synopsis

  \desc \b \this posts \a n \a messages to the message queue pointed to by \a m.
  
  \b \this returns the number of messages that were posted.
*/
unsigned ctl_message_queue_post_multi_nb(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                         unsigned n,             // the number of messages to post
                                         void **messages);       // the message to post


/*! \brief Receive messages from a message queue \synopsis

  \desc \b \this pops the oldest \a n messages in the message queue pointed to
  by \a q into the memory pointed to by \a message. \b \this
  will block until all the messages are available unless \a timeoutType
  is non-zero and the current time reaches the \a timeout value. 

  \b \this returns the number of messages that were received.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_message_queue_receive_multi(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                         unsigned n,             // the number of messages to receive
                                         void **messages,        // pointer to message array
                                         CTL_TIMEOUT_t t,        // type of timeout
                                         CTL_TIME_t timeout);    // timeout


/*! \brief Receive messages from a message queue without blocking \synopsis

  \desc \b \this pops the oldest \a n messages in the message queue pointed to
  by \a q into the memory pointed to by \a message. 

  \b \this returns the number of messages that were received.  
*/
unsigned ctl_message_queue_receive_multi_nb(CTL_MESSAGE_QUEUE_t *q, // message queue pointer
                                            unsigned n,             // the number of messages to receive
                                            void **messages);       // pointer to message array

/*! \endgroup */


//
// Byte queue support - same as message queue but specialized for bytes
//
/*! \group Bytes queues */

/*! \brief Byte queue struct definition \ingroup Types \synopsis
  
  \desc \b \this defines the byte queue structure. The byte queue structure contains:

  \table{1.5in,*}
  || Member            | Description
  || q                 | pointer to the array of bytes
  || s                 | size of the array of bytes
  || front             | the next byte to leave the byte queue
  || n                 | the number of elements in the byte queue
  || e                 | the event set to use for the not empty and not full events
  || notempty          | the event number for a not empty event
  || notfull           | the event number for a not full event
  \endtable
*/
typedef struct {
  unsigned char *q;
  unsigned s;
  unsigned front;
  unsigned n;
  CTL_EVENT_SET_t *e;
  CTL_EVENT_SET_t notempty;
  CTL_EVENT_SET_t notfull;
} CTL_BYTE_QUEUE_t;


/*! \brief Initialize a byte queue \synopsis

  \desc \b \this is given a pointer to 
  the byte queue to initialize in \a q. The array that will be 
  used to implement the byte queue pointed to by \a queue and its size 
  in \a queue_size are also supplied.
*/
void ctl_byte_queue_init(CTL_BYTE_QUEUE_t *q,    // byte queue pointer
                         unsigned char *queue,   // pointer to the queue
                         unsigned queue_size);   // the number of bytes in the queue


/*! \brief Associate events with the not-full and not-empty state of a byte queue \synopsis

  \desc \b \this registers events in the event set \a e that are set
  when the byte queue \a q becomes \a notempty or becomes \a notfull.
  No scheduling will occur with this operation, you need to do this
  before waiting for events.
*/
void ctl_byte_queue_setup_events(CTL_BYTE_QUEUE_t *q, // byte queue pointer
                                 CTL_EVENT_SET_t *e, // event set
                                 CTL_EVENT_SET_t notempty,
                                 CTL_EVENT_SET_t notfull);


/*! \brief Return number of free bytes in a byte queue \synopsis

  \desc \b \this returns the number of free bytes in the byte queue \b q.
*/
unsigned ctl_byte_queue_num_free(CTL_BYTE_QUEUE_t *q);


/*! \brief Return number of used bytes in a byte queue \synopsis

  \desc \b \this returns the number of used elements in the byte queue \b q.
*/
unsigned ctl_byte_queue_num_used(CTL_BYTE_QUEUE_t *q);


/*! \brief Post byte to a byte queue \synopsis

  \desc \b \this posts \a b to the byte queue pointed to
  by \a q. If the byte queue is full then the caller will block until
  the byte can be posted or, if \a timeoutType is non-zero, the
  current time reaches \a timeout value.
  
  \b \this returns zero if the timeout occurred otherwise it returns one.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_byte_queue_post(CTL_BYTE_QUEUE_t *q,    // byte queue pointer
                             unsigned char b,        // the byte to post
                             CTL_TIMEOUT_t t,        // type of timeout
                             CTL_TIME_t timeout);    // timeout


/*! \brief Post byte to a byte queue without blocking \synopsis

  \desc \b \this posts \a b to the byte queue pointed to
  by \a q. If the byte queue is full then the function will return
  zero otherwise it will return one.
*/
unsigned ctl_byte_queue_post_nb(CTL_BYTE_QUEUE_t *q,  // byte queue pointer
                                unsigned char b);     // the byte to post


/*! \brief Receive a byte from a byte queue \synopsis

  \desc \b \this pops the oldest byte in the byte queue pointed to
  by \a q into the memory pointed to by \a b. \b \this
  will block if no bytes are available unless \a timeoutType
  is non-zero and the current time reaches the \a timeout value. 

  \b \this returns zero if a timeout occurs otherwise 1.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_byte_queue_receive(CTL_BYTE_QUEUE_t *q,    // byte queue pointer
                                unsigned char *b,       // pointer to byte receiver
                                CTL_TIMEOUT_t t,        // type of timeout
                                CTL_TIME_t timeout);    // timeout


/*! \brief Receive a byte from a byte queue without blocking \synopsis

  \desc \b \this pops the oldest byte in the byte queue pointed to
  by \a m into the memory pointed to by \a b. If no bytes are available 
  the function returns zero otherwise it returns 1.
*/
unsigned ctl_byte_queue_receive_nb(CTL_BYTE_QUEUE_t *q, // byte queue pointer
                                   unsigned char *b);   // pointer to byte receiver


/*! \brief Post bytes to a byte queue \synopsis

  \desc \b \this posts \a n bytes to the byte queue pointed to
  by \a q. The caller will block until
  the bytes can be posted or, if \a timeoutType is non-zero, the
  current time reaches \a timeout value.
  
  \b \this returns the number of bytes that were posted.

  \note \b \this must not be called from an interrupt service routine.

  \b \this does not guarantee that the bytes will be all be posted to the byte queue atomically. 
  If you have multiple tasks posting (multiple bytes) to the same byte queue then you may get unexpected results.
 
*/
unsigned ctl_byte_queue_post_multi(CTL_BYTE_QUEUE_t *q,    // byte queue pointer
                                   unsigned n,             // the number of bytes to post
                                   unsigned char *b,       // the bytes to post
                                   CTL_TIMEOUT_t t,        // type of timeout
                                   CTL_TIME_t timeout);    // timeout


/*! \brief Post bytes to a byte queue without blocking \synopsis

  \desc \b \this posts \a n bytes to the byte queue pointed to by \a q.
  
  \b \this returns the number of bytes that were posted.
*/
unsigned ctl_byte_queue_post_multi_nb(CTL_BYTE_QUEUE_t *q,  // byte queue pointer
                                      unsigned n,           // the number of bytes to post
                                      unsigned char *b);    // the bytes to post


/*! \brief Receive multiple bytes from a byte queue \synopsis

  \desc \b \this pops the oldest \a n bytes in the byte queue
  pointed to by \a q into the memory pointed at by \a b. \b \this will
  block until the number of bytes are available unless \a timeoutType
  is non-zero and the current time reaches the \a timeout value.

  \b \this returns the number of bytes that have been received.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_byte_queue_receive_multi(CTL_BYTE_QUEUE_t *q,    // byte queue pointer
                                      unsigned n,             // the number of bytes to receive
                                      unsigned char *b,       // pointer to byte array
                                      CTL_TIMEOUT_t t,        // type of timeout
                                      CTL_TIME_t timeout);    // timeout


/*! \brief Receive multiple bytes from a byte queue without blocking \synopsis

  \desc \b \this pops the oldest \a n bytes in the byte queue pointed to by \a q
  into the memory pointed to by \a b. 
  
  \b \this returns the number of bytes that have been received.
*/
unsigned ctl_byte_queue_receive_multi_nb(CTL_BYTE_QUEUE_t *q, // byte queue pointer
                                         unsigned n,          // the number of bytes to receive
                                         unsigned char *b);   // pointer to byte array

/*! \endgroup */

//
// Mutex support
//

/*! \group Mutexes */


/*! \brief Mutex struct definition \ingroup Types \synopsis
  
  \desc \b \this defines the mutex structure. The mutex structure contains:

  \table{1.5in,*}
  || Member                | Description
  || lock_count            | number of times the mutex has been locked
  || locking_task          | the task that has locked the mutex
  || locking_task_priority | the priority of the task at the time it locked the mutex
  \endtable
*/
typedef struct {
  unsigned lock_count;
  CTL_TASK_t *locking_task;
  unsigned locking_task_priority;
} CTL_MUTEX_t;


/*! \brief Initialize a mutex \synopsis

  \desc \b \this initializes the mutex pointed to by \a m.
*/
void ctl_mutex_init(CTL_MUTEX_t *m);


/*! \brief Unlock a mutex \synopsis

  \desc \b \this unlocks the mutex pointed to by \a m. The mutex must
  have previously been locked by the calling task. If the calling task's
  priority has been raised (by another task calling \b \this whilst the mutex was locked),
  then the calling tasks priority will be restored.

  \note \b \this must not be called from an interrupt service routine.
*/
void ctl_mutex_unlock(CTL_MUTEX_t *m);


/*! \brief Lock a mutex \synopsis

  \desc \b \this locks the mutex pointed to by \a m to the calling task.
  If the mutex is already locked by the calling task then the mutex lock count is
  incremented.  If the mutex is already locked by a different task then the caller
  will block until the mutex is unlocked.  In this case, if the priority of the task
  that has locked the mutex is less than that of the caller the priority of the task
  that has locked the mutex is raised to that of the caller whilst the mutex
  is locked.
  
  If \a timeoutType is non-zero and the current time reaches the \a timeout value
  before the lock is acquired the function returns zero otherwise it returns one.

  \note \b \this must not be called from an interrupt service routine.
*/
unsigned ctl_mutex_lock(CTL_MUTEX_t *m,
                        CTL_TIMEOUT_t t,
                        CTL_TIME_t timeout);


/*! \endgroup Mutex */


//
// Global interrupts support
//
/*! \group Interrupts */


/*! \brief Enable/disable interrupts \synopsis

  \desc \b \this disables or enables global interrupts according 
  to the state \b enable. If \b enable is zero, interrupts are disabled 
  and if \b enable is non-zero, interrupts are enabled. If \this is called
  and interrupts are already disabled then it will return 0. If \this is called
  and interrupts are enabled then it will return non-zero which may or may not represent the
  true interrupt disabled state. \this is used to provide exclusive access to CTL
  data structures the implementation of it may or may not disable global interrupts.
*/
int ctl_global_interrupts_set(int enable);


#if defined(__CROSSWORKS_DOCUMENTATION) || defined(_lint)


/*! \brief Disable global interrupts \synopsis

  \desc \b \this disables global interrupts. If \this is called and interrupts are already 
  disabled then it will return 0. If \this is called and interrupts are enabled then it will 
  return non-zero which may or may not represent the true interrupt disabled state. 
  \this is used to provide exclusive access to CTL data structures the implementation of 
  it may or may not disable global interrupts.
*/
int ctl_global_interrupts_disable(void);


/*! \brief Enable global interrupts \synopsis

  \desc \b \this enables global interrupts. \this is used to provide exclusive access 
  to CTL data structures the implementation of it may or may not disable global interrupts.
*/
void ctl_global_interrupts_enable(void);

#else

#if !defined(__NO_USE_INTRINSICS__) && defined(__CROSSWORKS_ARM) && ((defined(__ARM_ARCH_4T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_6__)) && !defined(__thumb__)) || defined(__ARM_ARCH_6T2__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__)

#include <intrinsics.h>

#define ctl_global_interrupts_disable()\
  (__disable_irq() == 0)

#define ctl_global_interrupts_enable()\
  __enable_irq()

#elif !defined(__NO_USE_INTRINSICS__) && defined(__CROSSWORKS_ARM) && defined(__ARM_ARCH_6M__)

#define ctl_global_interrupts_disable()\
({\
  int __primask;\
  __asm__ __volatile__("mrs %[__primask], primask\n"\
                       "cpsid i\n"\
                       : [__primask] "=r" (__primask));\
  (__primask == 0);\
})
#define ctl_global_interrupts_enable()\
({\
  __asm__ __volatile__("cpsie i\n");\
})

#elif !defined(__NO_USE_INTRINSICS__) && defined(__CROSSWORKS_ARM) && (defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7EM__))

#define ctl_global_interrupts_disable()\
({\
  int __basepri, __tmp;\
  __asm__ __volatile__("mrs %[__basepri], basepri\n"\
                       "mov %[__tmp], #0x80\n"\
                       "msr basepri, %[__tmp]\n"\
                       : [__basepri] "=r" (__basepri), [__tmp] "=r" (__tmp));\
  ((__basepri & 0x80) == 0);\
})

#define ctl_global_interrupts_enable()\
({\
 int __tmp;\
  __asm__ __volatile__("mov %[__tmp], #0x00\n"\
                       "msr basepri, %[__tmp]\n"\
                       : [__tmp] "=r" (__tmp));\
})

#elif !defined(__NO_USE_INTRINSICS__) && defined(__CROSSWORKS_AVR)

#include <inavr.h>

#define ctl_global_interrupts_disable()\
  (__bic_SR_register(0x80) & 0x80)

#define ctl_global_interrupts_enable()\
  __bis_SR_register(0x80)

#elif !defined(__NO_USE_INTRINSICS__) && defined(__CROSSWORKS_MSP430)

#include <inmsp.h>

#define ctl_global_interrupts_disable()\
  (__bic_SR_register(8) & (8))

#define ctl_global_interrupts_enable()\
  __bis_SR_register(8)

#else 

#define ctl_global_interrupts_disable() ctl_global_interrupts_set(0)
#define ctl_global_interrupts_enable() ctl_global_interrupts_set(1)

#endif
#endif

/*! \endgroup */


//
// Error handling
//
/*! \group Error handling */

/*! \brief Error cause \ingroup Types \synopsis

  \desc \b \this defines the set of errors that are detected by the
  CrossWorks tasking library; the errors are reported by a call
  to \b ctl_handle_error.

  \table{1.5in,*}
  || Constant | Description
  || CTL_ERROR_NO_TASKS_TO_RUN |
       A reschedule has occurred but there are no tasks which are runnable.
  || CTL_UNSUPPORTED_CALL_FROM_ISR |
       An interrupt service routine has called a tasking library function
       that could block or is otherwise unsupported when called from inside
       an interrupt service routine.
  || CTL_MUTEX_UNLOCK_CALL_ERROR |
       A task called \a ctl_mutex_unlock passing a mutex which it has not
       locked, or which a different task holds a lock on.  Only the task
       that successfully acquired a lock on a mutex can unlock that mutex.
  || CTL_UNSPECIFIED_ERROR |
       An unspecified error has occurred.
  \endtable
*/
typedef enum
{
  CTL_ERROR_NO_TASKS_TO_RUN,
  CTL_UNSUPPORTED_CALL_FROM_ISR,
  CTL_MUTEX_UNLOCK_CALL_ERROR,
  CTL_UNSPECIFIED_ERROR
} CTL_ERROR_CODE_t;


/*! \brief Handle a CTL error condition \synopsis

  \desc \b \this is a function that you must supply in your application
  that handles errors detected by the CrossWorks tasking library.

  The errors that can be reported in \a e are are described in
  \ref CTL_ERROR_CODE_t.
*/
void ctl_handle_error(CTL_ERROR_CODE_t __e);

/*! \endgroup */

//
// Memory area support
//
/*! \group Memory areas */


/*! \brief Memory area struct definition \ingroup Types \synopsis
  
  \desc \b \this defines the memory area structure. The memory area structure contains:

  \table{1.5in,*}
  || Member                | Description
  || head                  | the next free memory block
  || e                     | the event set containing the blockavailable event
  || blockavailable        | the blockavailable event
  \endtable
*/
typedef struct {
  unsigned *head;
  CTL_EVENT_SET_t *e;
  CTL_EVENT_SET_t blockavailable;
} CTL_MEMORY_AREA_t;


/*! \brief Initialize a memory area \synopsis

  \desc \b \this is given a pointer to the memory area to initialize
  in \a memory_area. The array that is used to implement the memory area is
  pointed to by \a memory. The size  of a memory block is given supplied in
  \a block_size_in_words and the number of block is supplied in \a num_blocks.
  
  \note \a memory must point to a block of memory that is at least
  \a block_size_in_words \times \a num_blocks words long.
*/
void ctl_memory_area_init(CTL_MEMORY_AREA_t *memory_area, // pointer to the memory area
                          unsigned *memory, // should be block_size_in_words * num_blocks in length
                          unsigned block_size_in_words, // size of the memory block in words
                          unsigned num_blocks); // the number of blocks


/*! \brief Set memory area events \synopsis

  \desc \b \this registers the events \a blockavailable in the
  event set \a e that are set when a block becomes available in the
  the memory area \a m.
*/
void ctl_memory_area_setup_events(CTL_MEMORY_AREA_t *m, // memory area pointer
                                  CTL_EVENT_SET_t *e, // event set
                                  CTL_EVENT_SET_t blockavailable);


/*! \brief Allocate a block from a memory area \synopsis

  \desc \b \this allocates a block from the initialized memory area \a memory_area.
  \b \this returns a block of the size specified in the call to \ref ctl_memory_area_init
  or zero if no blocks are available.

  \b \this executes in constant time and is very fast.  You can call \b \this from
  an interrupt service routine, from a task, or from initialization code.
*/
unsigned *ctl_memory_area_allocate(CTL_MEMORY_AREA_t *memory_area);


/*! \brief Free a memory area block \synopsis

  \desc \b \this is given a pointer to a memory area \a memory_area
  which has been initialized and a \a block that has been returned by \ref ctl_memory_area_allocate.
  The block is returned to the memory area so that it can be allocated again.
*/
void ctl_memory_area_free(CTL_MEMORY_AREA_t *memory_area,
                          unsigned *block);

/*! \endgroup */

//
// System state
//
/*! \group System state variables */


/*! \brief List of tasks sorted by priority \synopsis

  \desc \b \this points to the \b CTL_TASK_t structure of the highest priority task that
  is not executing. It is an error if \b \this is \b NULL.
*/
extern CTL_TASK_t *ctl_task_list;


/*! \brief The task that is currently executing \synopsis

  \desc \b \this points to the \b CTL_TASK_t structure of the currently
  executing task.  The \b priority field is the only field in the \b CTL_TASK_t
  structure that is defined for the task that is executing.  It is an error
  if \b \this is \b NULL.
*/
extern CTL_TASK_t *ctl_task_executing;


/*! \brief Nested interrupt count \synopsis

  \desc \b \this contains a count of the interrupt nesting level. This variable must
  be incremented immediately on entry to an interrupt service routine and decremented
  immediately before return from the interrupt service routine.
*/
extern unsigned char ctl_interrupt_count;


/*! \brief Reschedule is required on last ISR exit \synopsis

  \desc \b \this is set to a non-zero value if a CTL call is made from an interrupt
  service routine that requires a task reschedule. This variable is 
  checked and reset on exit from the last nested interrupt service routine.
*/
extern unsigned char ctl_reschedule_on_last_isr_exit;


/*! \brief The current time in ticks \synopsis

  \desc \b \this holds the current time in ticks. \b \this is incremented
  by \b ctl_increment_ticks_from_isr.

  \note For portable programs without race conditions you should not read this
  variable directly, you should use \b ctl_get_current_time instead.  As
  this variable is changed by an interrupt, it cannot be read atomically on
  processors whose word size is less than 32 bits without first disabling
  interrupts.  That said, you can read this variable directly in your interrupt
  handler as long as interrupts are still disabled.
*/
extern CTL_TIME_t ctl_current_time;


/*! \brief Time slice period in ticks \synopsis

  \desc \b \this contains the number of ticks to allow a task to run before it will be
  preemptively rescheduled by a task of the same priority. The variable 
  is set to zero by default so that only higher priority tasks will be preemptively
  scheduled.
*/
extern CTL_TIME_t ctl_timeslice_period;


/*! \brief Current time tick increment \synopsis

  \desc \b \this contains the value that \b ctl_current_time is
  incremented when \b ctl_increment_tick_from_isr is called.
*/
extern unsigned ctl_time_increment;


/*! \brief The time (in ticks) of the last task schedule \synopsis

  \desc \b \this contains the time (in ticks) of the last task schedule.

  \desc \b \this contains the time of the last reschedule in ticks.
*/
extern CTL_TIME_t ctl_last_schedule_time;


/*! \brief A function pointer called on a task switch \synopsis

  \desc \b \this contains a pointer to a function that is called (if it is set) when a
  task schedule occurs. The task that will be scheduled is supplied as a parameter to the
  function (\b ctl_task_executing will point to the currently scheduled task). 

  Note that the callout function is called from the CTL scheduler and as such any use of CTL
  services whilst executing the callout function has undefined behavior.

  \note Because this function pointer is called in an interrupt service routine,
  you should assign it before interrupts are started or with interrupts turned off.
*/
extern void (*ctl_task_switch_callout)(CTL_TASK_t *);

/*! \endgroup */


#ifdef __cplusplus
}
#endif

#endif
