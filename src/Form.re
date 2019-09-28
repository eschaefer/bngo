[@react.component]
let make = (~handleSubmit, ~isLoading) => {
  let (username, setUsername) = React.useState(() => "");

  <form
    className="pa3 bb"
    onSubmit={event => {
      ReactEvent.Form.preventDefault(event);
      handleSubmit(username);
    }}>
    <h1 className="f4">
      {React.string("Explore favorite tracks from the people you follow.")}
    </h1>
    <label>
      {React.string("Soundcloud user name:")}
      <input
        className="input-reset ba bg-transparent pa2 b--black mh3"
        type_="text"
        value=username
        placeholder="i.e call-super"
        onChange={event => setUsername(ReactEvent.Form.target(event)##value)}
      />
    </label>
    <input
      className="bn f6 dim ph3 pv2 mb2 outline-0 white bg-black pointer"
      type_="submit"
      disabled=isLoading
      value={isLoading ? "Loading..." : "Submit"}
    />
  </form>;
};